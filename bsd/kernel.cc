//
//  NetBSD port:
//  Copyright (c) 1995, 1996, 1997-2002, 2015, 2016
//  by Brian Grayson (bgrayson@netbsd.org)
//
//  This file was written by Brian Grayson for the NetBSD and xosview
//    projects.
//  This file contains code from the NetBSD project, which is covered
//    by the standard BSD license.
//  Dummy device ignore code by : David Cuka (dcuka@intgp1.ih.att.com)
//  The OpenBSD interrupt meter code was written by Oleg Safiullin
//    (form@vs.itam.nsc.ru).
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//
#include "kernel.h"
#include "log.h"
#include "strutil.h"
#include "sctl.h"

#include <cstring>
#include <cerrno>

#include <unistd.h>
#include <fcntl.h>
#include <limits.h>

#include <sys/param.h>
#include <sysexits.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <sys/socket.h>


#if defined(XOSVIEW_DFBSD)
#define _KERNEL_STRUCTURES
#include <kinfo.h>
#endif


#if defined(XOSVIEW_FREEBSD) || defined(XOSVIEW_DFBSD)
static const char * const ACPIDEV = "/dev/acpi";
static const char * const APMDEV = "/dev/apm";

#include <net/if.h>
#include <net/if_var.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <dev/acpica/acpiio.h>
#include <machine/apm_bios.h>
#include <net/if_mib.h>
#endif


#if defined(XOSVIEW_NETBSD)
#include <sys/sched.h>
#include <sys/iostat.h>
#include <sys/envsys.h>
#include <prop/proplib.h>
#include <paths.h>
#if defined(XOSV_NETBSD_NET_IOCTL)
#include <net/if.h>
#include <sys/ioctl.h>
#include <machine/int_fmtio.h>
#endif
#endif


#if defined(XOSVIEW_OPENBSD)
#include <sys/sched.h>
#include <sys/disk.h>
#if defined(HAVE_SYS_DKSTAT_H)
#include <sys/dkstat.h>
#elif defined(HAVE_SYS_SCHED_H)
#include <sys/sched.h>
#endif
#include <sys/mount.h>
#include <net/route.h>
#include <net/if_dl.h>
#endif


#if defined(XOSVIEW_OPENBSD) || defined(XOSVIEW_DFBSD)
#include <sys/sensors.h>
#endif

#if defined(HAVE_DEVSTAT)
#include <devstat.h>
#endif

#if defined(HAVE_UVM)
#include <sys/device.h>
#include <uvm/uvm_extern.h>
#else
#include <sys/vmmeter.h>
#endif

#if defined(HAVE_SWAPCTL)
#include <sys/swap.h>
#endif


// Number of elements in a static array.
#define ASIZE(ar) (sizeof(ar) / sizeof(ar[0]))


// --------------------  PageMeter & MemMeter functions  -----------------------

/* meminfo[5]  = { active, inactive, wired, cached, free } */
/* pageinfo[2] = { pages_in, pages_out }                   */
void BSDGetMemPageStats(std::vector<uint64_t> &meminfo,
  std::vector<uint64_t> &pageinfo) {
#if defined(HAVE_UVM)
#ifdef VM_UVMEXP2
    struct uvmexp_sysctl uvm;
    const int mib_uvm[] = { CTL_VM, VM_UVMEXP2 };
#else
    struct uvmexp uvm;
    const int mib_uvm[] = { CTL_VM, VM_UVMEXP };
#endif
    size_t size = sizeof(uvm);
    if ( sysctl(mib_uvm, ASIZE(mib_uvm), &uvm, &size, NULL, 0) < 0 )
        logFatal << "sysctl vm.uvmexp failed" << std::endl;

    meminfo.resize(5);
    // UVM excludes kernel memory -> assume it is active mem
    meminfo[0] = (uint64_t)(uvm.npages - uvm.inactive - uvm.wired
      - uvm.free) * uvm.pagesize;
    meminfo[1] = (uint64_t)uvm.inactive * uvm.pagesize;
    meminfo[2] = (uint64_t)uvm.wired * uvm.pagesize;

    // cache is already included in active and inactive memory and
    // there's no way to know how much is in which -> disable cache
    meminfo[3] = 0;
    meminfo[4] = (uint64_t)uvm.free * uvm.pagesize;

    pageinfo.resize(2);
    pageinfo[0] = (uint64_t)uvm.pgswapin;
    pageinfo[1] = (uint64_t)uvm.pgswapout;

#else  /* HAVE_UVM */
    struct vmmeter vm;
#if defined(XOSVIEW_FREEBSD)
    size_t size = 0;
#define	GET_VM_STATS(name)                                                \
    size = sizeof(vm.name);                                               \
    if (sysctlbyname("vm.stats.vm." #name, &vm.name, &size, NULL, 0) < 0) \
        logFatal << "sysctlbyname(vm.stats.vm." #name << ") failed.\n";

    GET_VM_STATS(v_active_count);
    GET_VM_STATS(v_inactive_count);
    GET_VM_STATS(v_wire_count);
    GET_VM_STATS(v_cache_count);
    GET_VM_STATS(v_free_count);
    GET_VM_STATS(v_page_size);
    GET_VM_STATS(v_vnodepgsin);
    GET_VM_STATS(v_vnodepgsout);
    GET_VM_STATS(v_swappgsin);
    GET_VM_STATS(v_swappgsout);
#undef GET_VM_STATS

#else  /* XOSVIEW_DFBSD */
    struct vmstats vms;
    size_t size = sizeof(vms);
    if ( sysctlbyname("vm.vmstats", &vms, &size, NULL, 0) < 0 )
        logFatal << "sysctl vm.vmstats failed" << std::endl;
    size = sizeof(vm);
    if ( sysctlbyname("vm.vmmeter", &vm, &size, NULL, 0) < 0 )
        logFatal << "sysctl vm.vmmeter failed" << std::endl;
#endif

    meminfo.resize(5);
#if defined(XOSVIEW_FREEBSD)
    meminfo[0] = (uint64_t)vm.v_active_count * vm.v_page_size;
    meminfo[1] = (uint64_t)vm.v_inactive_count * vm.v_page_size;
    meminfo[2] = (uint64_t)vm.v_wire_count * vm.v_page_size;
    meminfo[3] = (uint64_t)vm.v_cache_count * vm.v_page_size;
    meminfo[4] = (uint64_t)vm.v_free_count * vm.v_page_size;
#else  /* XOSVIEW_DFBSD */
    meminfo[0] = (uint64_t)vms.v_active_count * vms.v_page_size;
    meminfo[1] = (uint64_t)vms.v_inactive_count * vms.v_page_size;
    meminfo[2] = (uint64_t)vms.v_wire_count * vms.v_page_size;
    meminfo[3] = (uint64_t)vms.v_cache_count * vms.v_page_size;
    meminfo[4] = (uint64_t)vms.v_free_count * vms.v_page_size;
#endif

    pageinfo.resize(2);
    pageinfo[0] = (uint64_t)vm.v_vnodepgsin + (uint64_t)vm.v_swappgsin;
    pageinfo[1] = (uint64_t)vm.v_vnodepgsout + (uint64_t)vm.v_swappgsout;
#endif
}


// ------------------------  CPUMeter functions  -------------------------------

#if !defined(XOSVIEW_OPENBSD) && !defined(XOSVIEW_DFBSD)
static size_t BSDCountCpus(void) {

    static bool first = true;
    static size_t cpus = 0;

    if (first) {
        first = false;
        size_t size = sizeof(cpus);
        const int mib_cpu[2] = { CTL_HW, HW_NCPU };

        if ( sysctl(mib_cpu, 2, &cpus, &size, NULL, 0) < 0 )
            logProblem << "sysctl hw.ncpu failed." << std::endl;
    }

    return cpus;
}
#endif


void BSDGetCPUTimes(std::vector<uint64_t> &timeArray, unsigned int cpu) {
    // timeArray is CPUSTATES long.
    // cpu is the number of CPU to return, starting from 1. If cpu == 0,
    // return aggregate times for all CPUs.
    // All BSDs have separate calls for aggregate and separate times. Only
    // OpenBSD returns one CPU per call, others return all at once.
    size_t size;

#if defined(XOSVIEW_DFBSD)
    int maxcpus = 1;
    if ( kinfo_get_cpus(&maxcpus) )
        logFatal << "kinfo_get_cpus() failed" << std::endl;
    size = sizeof(struct kinfo_cputime);
    std::vector<struct kinfo_cputime> times(maxcpus + 1, kinfo_cputime());
#elif defined(XOSVIEW_FREEBSD)
    int maxcpus = 1;
    size = sizeof(maxcpus);
    if ( sysctlbyname("kern.smp.maxcpus", &maxcpus, &size, NULL, 0) < 0 )
        logFatal << "sysctl kern.smp.maxcpus failed" << std::endl;
    size = CPUSTATES * sizeof(long);
    std::vector<long> times(maxcpus + 1, 0);
#elif defined(XOSVIEW_NETBSD)
    size = CPUSTATES * sizeof(uint64_t);
    std::vector<uint64_t> times((BSDCountCpus() + 1) * CPUSTATES, 0);
#elif defined(XOSVIEW_OPENBSD)
    std::vector<uint64_t> times(CPUSTATES, 0);
#endif
    // this array will have aggregate values at 0, then each CPU (except on
    // OpenBSD), so that cpu can be used as index

#if defined(XOSVIEW_DFBSD)
    if (cpu == 0) {
        if (kinfo_get_sched_cputime(times.data()))
            logFatal << "kinfo_get_sched_cputime() failed" << std::endl;
    }
    else {
        size = maxcpus * sizeof(times[0]);
        if ( sysctlbyname("kern.cputime", &times[1], &size, NULL, 0) < 0 )
            logFatal << "sysctl kern.cputime failed" << std::endl;
    }
    timeArray.resize(5);
    timeArray[0] = times[cpu].cp_user;
    timeArray[1] = times[cpu].cp_nice;
    timeArray[2] = times[cpu].cp_sys;
    timeArray[3] = times[cpu].cp_intr;
    timeArray[4] = times[cpu].cp_idle;
#else  // !XOSVIEW_DFBSD
    size = CPUSTATES * sizeof(times[0]);
    if (cpu == 0) {  // aggregate times
#if defined(XOSVIEW_FREEBSD)
        if ( sysctlbyname("kern.cp_time", times.data(), &size, NULL, 0) < 0 )
#else  // XOSVIEW_NETBSD || XOSVIEW_OPENBSD
#if defined(XOSVIEW_NETBSD)
        const int mib_cpt[] = { CTL_KERN, KERN_CP_TIME };
#else
        const int mib_cpt[] = { CTL_KERN, KERN_CPTIME };
#endif
        if ( sysctl(mib_cpt, ASIZE(mib_cpt), times.data(), &size, NULL, 0) < 0 )
#endif
            logFatal << "sysctl kern.cp_time failed" << std::endl;
    }
    else {  // separate times
#if defined(XOSVIEW_FREEBSD)
        size *= maxcpus;

        if ( sysctlbyname("kern.cp_times", times.data() + CPUSTATES, &size,
            NULL, 0) < 0 )
            logFatal << "sysctl kern.cp_times failed" << std::endl;

#elif defined(XOSVIEW_NETBSD)
        size *= BSDCountCpus();
        const int mib_cpt[] = { CTL_KERN, KERN_CP_TIME };
        if ( sysctl(mib_cpt, ASIZE(mib_cpt), times.data() + CPUSTATES, &size,
            NULL, 0) < 0 )
            logFatal << "sysctl kern.cp_time failed" << std::endl;
#else  // XOSVIEW_OPENBSD
        int mib_cpt2[] = { CTL_KERN, KERN_CPTIME2, (int)cpu - 1 };
        if (sysctl(mib_cpt2, ASIZE(mib_cpt2), times.data(), &size, NULL, 0) < 0)
            logFatal << "sysctl kern.cp_time2 failed" << std::endl;
#endif
    }
    timeArray.resize(CPUSTATES);
    for (int i = 0; i < CPUSTATES; i++)
#if defined(XOSVIEW_OPENBSD) // aggregates are long, singles uint64_t
        timeArray[i] = ( cpu ? times[i] : ((long*)(times.data()))[i] );
#else  // XOSVIEW_FREEBSD || XOSVIEW_NETBSD
        timeArray[i] = times[cpu * CPUSTATES + i];
#endif
#endif
}


//  ---------------------- Swap Meter stuff  -----------------------------------

void BSDGetSwapInfo(uint64_t &total, uint64_t &used) {
#if defined(HAVE_SWAPCTL)
    //  This code is based on a patch sent in by Scott Stevens
    //  (s.k.stevens@ic.ac.uk, at the time).
    int rnswap, nswap = swapctl(SWAP_NSWAP, 0, 0);
    total = used = 0;

    if (nswap < 1)  // no swap devices on
        return;

    std::vector<struct swapent> sep(nswap);

    rnswap = swapctl(SWAP_STATS, (void *)sep.data(), nswap);
    if (rnswap < 0)
        logFatal << "BSDGetSwapInfo(): getting SWAP_STATS failed" << std::endl;
    if (nswap != rnswap)
        logProblem << "SWAP_STATS gave different value than SWAP_NSWAP "
                   << "(nswap=" << nswap << " versus "
                   << "rnswap=" << rnswap << std::endl;

    // block size is that of underlying device, *usually* 512 bytes
    const int bsize = 512;
    for (size_t i = 0 ; i < (size_t)rnswap ; i++) {
        total += (uint64_t)sep[i].se_nblks * bsize;
        used += (uint64_t)sep[i].se_inuse * bsize;
    }
#elif defined(XOSVIEW_DFBSD)
    static SysCtl swsize_sc("vm.swap_size");
    static SysCtl swanon_sc("vm.swap_anon_use");
    static SysCtl swcache_sc("vm.swap_cache_use");

    size_t pagesize = getpagesize();
    int ssize = 0;
    int anon = 0;
    int cache = 0;

    if (!swsize_sc.get(ssize))
        logFatal << "sysctl(" << swsize_sc.id() << ") failed." << std::endl;
    if (!swanon_sc.get(anon))
        logFatal << "sysctl(" << swanon_sc.id() << ") failed." << std::endl;
    if (!swcache_sc.get(cache))
        logFatal << "sysctl(" << swcache_sc.id() << ") failed." << std::endl;

    total = ssize * pagesize;
    used = (anon + cache) * pagesize;

#elif defined(XOSVIEW_FREEBSD)

    static SysCtl nswaps_sc("vm.nswapdev");
    static SysCtl swapinfo_sc("vm.swap_info");
    if (swapinfo_sc.mib().size() == 2)
        swapinfo_sc.mib().push_back(0);

    int nswaps = 0;
    if (!nswaps_sc.get(nswaps))
        logFatal << "sysctl(" << nswaps_sc.id() << ") failed." << std::endl;

    // This thing is neither documented or even defined in a system
    // header.  We use it here because it is not KVM.  If it breaks then
    // FreeBSD does not get a swap meter until it has some sane way
    // to get the stats.
    //
    // Instead of the following an array of five ints will be used.
    //
    // struct xswdev {
    //     u_int   xsw_version;
    //     dev_t   xsw_dev;
    //     int     xsw_flags;
    //     int     xsw_nblks;
    //     int     xsw_used;
    // };

    size_t psize = getpagesize();

    std::vector<int> xswd(5, 0);
    for (int i = 0 ; i < nswaps ; i++) {
        swapinfo_sc.mib()[2] = i;
        if (!swapinfo_sc.get(xswd))
            logFatal << "sysctl(" << swapinfo_sc.id() << ") failed."
                     << std::endl;

        total += xswd[3] * psize;
        used += xswd[4] * psize;
    }

#else
#warning "swap stats method unknown."
    logBug << "swap stats unknown." << std::endl;
    total = 1;
    used = 0;
#endif
}


// ----------------------- Disk Meter stuff  -----------------------------------

#ifdef HAVE_DEVSTAT
/*
 * Make use of the new FreeBSD kernel device statistics library using
 * code shamelessly borrowed from xsysinfo, which borrowed shamelessly
 * from FreeBSD's iostat(8).
 */
long generation;
devstat_select_mode select_mode;
struct devstat_match *matches;
int num_matches = 0;
int num_selected, num_selections;
long select_generation;
static struct statinfo cur, last;
int num_devices;
struct device_selection *dev_select;
int nodisk = 0;


static void DevStat_Init(void) {
    /*
     * Make sure that the userland devstat version matches the kernel
     * devstat version.
     */
    if (
#if defined(XOSVIEW_FREEBSD)
        devstat_checkversion(NULL)
#else
        checkversion()
#endif
        < 0) {
        nodisk++;
        logProblem << devstat_errbuf << std::endl;
        return;
    }

    /* find out how many devices we have */
    if ( (num_devices =
#if defined(XOSVIEW_FREEBSD)
        devstat_getnumdevs(NULL)
#else
        getnumdevs()
#endif
          ) < 0 ) {
        nodisk++;
        logProblem << devstat_errbuf << std::endl;
        return;
    }

    cur.dinfo = (struct devinfo *)calloc(1, sizeof(struct devinfo));
    last.dinfo = (struct devinfo *)calloc(1, sizeof(struct devinfo));

    // Grab all the devices.  We don't look to see if the list has
    // changed here, since it almost certainly has.  We only look for
    // errors.
    if (
#if defined(XOSVIEW_FREEBSD)
        devstat_getdevs(NULL, &cur)
#else
        getdevs(&cur)
#endif
        == -1) {
        nodisk++;
        logProblem << devstat_errbuf << std::endl;
        return;
    }

    num_devices = cur.dinfo->numdevs;
    generation = cur.dinfo->generation;
    dev_select = NULL;

    /* only interested in disks */
    matches = NULL;
    char da[3] = "da";
    if (
#if defined(XOSVIEW_FREEBSD)
        devstat_buildmatch(da, &matches, &num_matches)
#else
        buildmatch(da, &matches, &num_matches)
#endif
        != 0) {
        nodisk++;
        logProblem << devstat_errbuf;
        return;
    }

    if (num_matches == 0)
        select_mode = DS_SELECT_ADD;
    else
        select_mode = DS_SELECT_ONLY;

    // At this point, selectdevs will almost surely indicate that the
    // device list has changed, so we don't look for return values of 0
    // or 1.  If we get back -1, though, there is an error.
    if (
#if defined(XOSVIEW_FREEBSD)
        devstat_selectdevs
#else
        selectdevs
#endif
        (&dev_select, &num_selected,
          &num_selections, &select_generation,
          generation, cur.dinfo->devices, num_devices,
          matches, num_matches, NULL, 0, select_mode, 10, 0) == -1) {
        nodisk++;
        logProblem << devstat_errbuf << std::endl;
    }
}


uint64_t DevStat_Get(uint64_t &read_bytes, uint64_t &write_bytes) {
    register int dn;
    long double busy_seconds;
    uint64_t reads, writes, total_bytes = 0;
    struct devinfo *tmp_dinfo;

    if (nodisk > 0)
        /* Diskless system or some error happened. */
        return 0;

    /*
     * Here what we want to do is refresh our device stats.
     * getdevs() returns 1 when the device list has changed.
     * If the device list has changed, we want to go through
     * the selection process again, in case a device that we
     * were previously displaying has gone away.
     */
#if defined(XOSVIEW_FREEBSD)
    switch (devstat_getdevs(NULL, &cur)) {
#else
    switch (getdevs(&cur)) {
#endif
    case -1:
        return (0);
    case 1:
        int retval;
        num_devices = cur.dinfo->numdevs;
        generation = cur.dinfo->generation;
#if defined(XOSVIEW_FREEBSD)
        retval = devstat_selectdevs(&dev_select, &num_selected,
#else
        retval = selectdevs(&dev_select, &num_selected,
#endif
          &num_selections, &select_generation,
          generation, cur.dinfo->devices,
          num_devices, matches, num_matches,
          NULL, 0, select_mode, 10, 0);
        switch(retval) {
        case -1:
            return (0);
        case 1:
            break;
        default:
            break;
        };
    default:
        break;
    };

    /*
     * Calculate elapsed time up front, since it's the same for all
     * devices.
     */
#if defined(XOSVIEW_FREEBSD)
    busy_seconds = cur.snap_time - last.snap_time;
#else
    busy_seconds = compute_etime(cur.busy_time, last.busy_time);
#endif
    /* this is the first time thru so just copy cur to last */
    if (last.dinfo->numdevs == 0) {
        tmp_dinfo = last.dinfo;
        last.dinfo = cur.dinfo;
        cur.dinfo = tmp_dinfo;
#if defined(XOSVIEW_FREEBSD)
        last.snap_time = cur.snap_time;
#else
        last.busy_time = cur.busy_time;
#endif
        return (0);
    }

    for (dn = 0; dn < num_devices; dn++) {
        int di;
        if ( (dev_select[dn].selected == 0) || (dev_select[dn].selected > 10) )
            continue;

        di = dev_select[dn].position;
#if defined(XOSVIEW_FREEBSD)
        if (devstat_compute_statistics(&cur.dinfo->devices[di],
            &last.dinfo->devices[di], busy_seconds,
            DSM_TOTAL_BYTES_READ, &reads,
            DSM_TOTAL_BYTES_WRITE, &writes,
            DSM_NONE) != 0) {
#else
        if (compute_stats_read(&cur.dinfo->devices[di],
            &last.dinfo->devices[di], busy_seconds,
            &reads, NULL,
            NULL, NULL, NULL, NULL, NULL, NULL) != 0) {
            logProblem << devstat_errbuf << std::endl;
            break;
        }
        if (compute_stats_write(&cur.dinfo->devices[di],
            &last.dinfo->devices[di], busy_seconds,
            &writes, NULL,
            NULL, NULL, NULL, NULL, NULL, NULL) != 0) {
#endif
            logProblem << devstat_errbuf << std::endl;
            break;
        }
        read_bytes += reads;
        write_bytes += writes;
        total_bytes += reads + writes;
    }

    tmp_dinfo = last.dinfo;
    last.dinfo = cur.dinfo;
    cur.dinfo = tmp_dinfo;
#if defined(XOSVIEW_FREEBSD)
    last.snap_time = cur.snap_time;
#else
    last.busy_time = cur.busy_time;
#endif

    return total_bytes;
}
#endif


bool BSDDiskInit() {
#if defined(HAVE_DEVSTAT)
    DevStat_Init();
#endif
    return true;
}


uint64_t BSDGetDiskXFerBytes(uint64_t &read_bytes, uint64_t &write_bytes) {
#if defined(HAVE_DEVSTAT)
    return DevStat_Get(read_bytes, write_bytes);
#else
    read_bytes = write_bytes = 0;
# if defined(XOSVIEW_NETBSD)
    size_t size;
    // Do a sysctl with a NULL data pointer to get the size that would
    // have been returned, and use that to figure out # drives.
    const int mib_dsk[] = { CTL_HW, HW_IOSTATS, sizeof(struct io_sysctl) };
    if ( sysctl(mib_dsk, ASIZE(mib_dsk), NULL, &size, NULL, 0) < 0 )
        logFatal << "BSDGetDiskXFerBytes(): sysctl hw.iostats #1 failed\n";

    unsigned int ndrives = size / mib_dsk[2];
    struct io_sysctl drive_stats[ndrives];

    // Get the stats.
    if ( sysctl(mib_dsk, ASIZE(mib_dsk), drive_stats, &size, NULL, 0) < 0 )
        logFatal << "BSDGetDiskXFerBytes(): sysctl hw.iostats #2 failed\n";

    // Now accumulate the total.
    for (uint i = 0; i < ndrives; i++) {
        read_bytes += drive_stats[i].rbytes;
        write_bytes += drive_stats[i].wbytes;
    }
# else  /* XOSVIEW_OPENBSD */

    const int mib1[] = {CTL_HW, HW_DISKCOUNT};
    int ndisks = 0;
    size_t dcsize = sizeof(ndisks);
    if (sysctl(mib1, 2, &ndisks, &dcsize, NULL, 0) < 0) {
        logFatal << "sysctl(CTL_HW, HW_DISKCOUNT) failed.\n";
    }

    const int mib2[] = {CTL_HW, HW_DISKSTATS};
    std::vector<struct diskstats> dstats(ndisks);
    size_t size = sizeof(struct diskstats) * dstats.size();
    if (sysctl(mib2, 2, dstats.data(), &size, NULL, 0) < 0) {
        logFatal << "sysctl(CTL_HW, HW_DISKSTATS) failed.\n";
    }

    for (size_t i = 0 ; i < dstats.size() ; i++) {
        read_bytes += dstats[i].ds_rbytes;
        write_bytes += dstats[i].ds_wbytes;
    }
# endif
#endif
    return (read_bytes + write_bytes);
}


//  ---------------------- Interrupt Meter stuff  ------------------------------

#if defined(XOSVIEW_DFBSD)
static int DFBSDNumInts(void) {

    static SysCtl intrnames_sc("hw.intrnames");

    size_t bytes = 0;
    if (!intrnames_sc.getsize(bytes))
        logFatal << "sysctl(" << intrnames_sc.id() << ") failed." << std::endl;

    std::vector<char> nbuf(bytes, 0);
    if (!intrnames_sc.get(nbuf))
        logFatal << "sysctl(" << intrnames_sc.id() << ") failed." << std::endl;

    size_t i = 0, maxirq = 0, offset = 0;
    while (offset < nbuf.size()) {
        // unused ints are named irqn where
        // 0<=n<=255, used ones have device name
        std::string tstr(nbuf.data() + offset);
        if (tstr.substr(0, 3) != "irq")
            maxirq = i;
        offset += tstr.size() + 1;
        i++;
    }

    return maxirq;
}
#endif


#if defined(XOSVIEW_FREEBSD)
static int FBSDNumInts(void) {

    static SysCtl intrnames_sc("hw.intrnames");

    int count = 0;

    size_t nsize = 0;
    if (!intrnames_sc.getsize(nsize))
        logFatal << "sysctl(" << intrnames_sc.id() << ") failed." << std::endl;

    std::vector<char> nbuf(nsize, 0);  // this thing will be 200kb or more!
    if (!intrnames_sc.get(nbuf))
        logFatal << "sysctl(" << intrnames_sc.id() << ") failed." << std::endl;

    const char *p = nbuf.data();
    // executive decision.  Stop at first null string
    // so we don't check 200k empty ones.
    while (*p && p < nbuf.data() + nbuf.size()) {
        std::string iname(p);
        std::istringstream is(iname);
        int nbr = 0;
        is >> util::sink("irq") >> nbr;
        if (is && nbr > count)
            count = nbr;
        p += iname.size() + 1;
    }

    return count;
}
#endif


#if defined(XOSVIEW_OPENBSD)
static int OBSDNumInts() {
    int count = 0;
    int nbr = 0;
    int nintr = 0;
    int mib_int[4] = { CTL_KERN, KERN_INTRCNT, KERN_INTRCNT_NUM };
    size_t size = sizeof(nintr);
    if ( sysctl(mib_int, 3, &nintr, &size, NULL, 0) < 0 ) {
        logProblem << "Could not get interrupt count" << std::endl;
        return 0;
    }
    for (int i = 0; i < nintr; i++) {
        mib_int[2] = KERN_INTRCNT_VECTOR;
        mib_int[3] = i;
        size = sizeof(nbr);
        if ( sysctl(mib_int, 4, &nbr, &size, NULL, 0) < 0 ) {
            logProblem << "Could not get name of interrupt " << i << std::endl;
        }
        else
            if ( nbr > count )
                count = nbr;
    }
    return count;
}
#endif


#if defined(XOSVIEW_NETBSD)
static int NBSDNumInts() {

    const int Mib[] = { CTL_KERN, KERN_EVCNT, EVCNT_TYPE_INTR,
                        KERN_EVCNT_COUNT_ANY };
    static SysCtl evcnt_sc(Mib, sizeof(Mib) / sizeof(int));

    int count = 0;

    size_t evsize = 0;
    if (!evcnt_sc.getsize(evsize))
        logFatal << "sysctl(" << evcnt_sc.id() << ") failed." << std::endl;

    std::vector<char> buf(evsize, 0);
    if (!evcnt_sc.get(buf))
        logFatal << "sysctl(" << evcnt_sc.id() << ") failed." << std::endl;

    const struct evcnt_sysctl *evs =
        reinterpret_cast<const struct evcnt_sysctl *>(buf.data());
    const struct evcnt_sysctl *evsend =
        reinterpret_cast<const struct evcnt_sysctl *>(buf.data() + buf.size());

    while (evs->ev_len && evs < evsend &&
      reinterpret_cast<const struct evcnt_sysctl *>(
          (char *)evs + evs->ev_len * 8) < evsend ) {

        // extract the "pin" number from the name.
        std::string name(evs->ev_strings + evs->ev_grouplen + 1);
        int nbr = 0;
        std::string dummy;
        std::istringstream is(name);
        is >> dummy >> nbr;

        if (is && nbr > count)
            count = nbr;

        evs = reinterpret_cast<const struct evcnt_sysctl *>((const char *)evs
          + 8 * evs->ev_len);
    }

    return count;
}
#endif


int BSDNumInts() {
    int count = 0;

#if defined(XOSVIEW_FREEBSD)
    count = FBSDNumInts();
#elif defined(XOSVIEW_NETBSD)
    count = NBSDNumInts();
#elif defined(XOSVIEW_OPENBSD)
    count = OBSDNumInts();
#elif defined(XOSVIEW_DFBSD)
    count = DFBSDNumInts();
#endif

    return count;  // this is the highest numbered interrupt
}


#if defined(XOSVIEW_DFBSD)
static void DFBSDGetIntrStats(std::vector<uint64_t> &intrCount,
  std::vector<unsigned int> &intrNbrs) {

    static SysCtl intrnames_sc("hw.intrnames");
    static SysCtl intrcnt_sc("hw.intrcnt");

    size_t bytes = 0;
    if (!intrnames_sc.getsize(bytes))
        logFatal << "sysctl(" << intrnames_sc.id() << ") failed." << std::endl;

    std::vector<char> nbuf(bytes, 0);
    if (!intrnames_sc.get(nbuf))
        logFatal << "sysctl(" << intrnames_sc.id() << ") failed." << std::endl;

    size_t csize = 0;
    if (!intrcnt_sc.getsize(csize))
        logFatal << "sysctl(" << intrcnt_sc.id() << ") failed." << std::endl;

    std::vector<unsigned long> intrcnt(csize / sizeof(unsigned long));
    if (!intrcnt_sc.get(intrcnt))
        logFatal << "sysctl(" << intrcnt_sc.id() << ") failed." << std::endl;

    size_t i = 0, offset = 0;
    while (i < intrCount.size() && offset < nbuf.size()) {
        // unused ints are named irqn where
        // 0<=n<=255, used ones have device name
        std::string tstr(nbuf.data() + offset);
        if (tstr.substr(0, 3) != "irq") {
            intrCount[i] += intrcnt[i];
            intrNbrs[i] = 1;
        }
        else {
            intrCount[i] = 0;
            intrNbrs[i] = 0;
        }
        offset += tstr.size() + 1;
        i++;
    }
}
#endif


#if defined(XOSVIEW_FREEBSD)
static void FBSDGetIntrStats(std::vector<uint64_t> &intrCount,
  std::vector<unsigned int> &intrNbrs) {

    static SysCtl intrcnt_sc("hw.intrcnt");
    static SysCtl intrnames_sc("hw.intrnames");

    int nbr = 0;

    size_t inamlen = 0;
    if (!intrnames_sc.getsize(inamlen))
        logFatal << "sysctl(" << intrnames_sc.id() << ") failed." << std::endl;
    std::vector<char> intrnames(inamlen);
    if (!intrnames_sc.get(intrnames))
        logFatal << "sysctl(" << intrnames_sc.id() << ") failed." << std::endl;

    size_t nintr = 0;
    if (!intrcnt_sc.getsize(nintr))
        logFatal << "sysctl(" << intrcnt_sc.id() << ") failed." << std::endl;
    std::vector<unsigned long> intrcnt(nintr / sizeof(unsigned long));
    if (!intrcnt_sc.get(intrcnt))
        logFatal << "sysctl(" << intrcnt_sc.id() << ") failed." << std::endl;


    nintr = intrcnt.size();
    // intrname has the ASCII names of the IRQs, every null-terminated
    // string corresponds to a value in the intrcnt array
    // e.g. irq1: atkbd0
    const char *p = intrnames.data();
    for (size_t i = 0 ; i < nintr ; i++) {
        // bail out on first null string as the list can be YUUUGE!
        if (!*p)
            break;

        /* Figure out which irq we have here */
        std::istringstream is(p);
        is >> util::sink("irq") >> nbr;
        if (is) {
            intrCount[nbr] = intrcnt[i];
            intrNbrs[nbr] = 1;
        }

        p += is.str().size() + 1;
    }
}
#endif


#if defined(XOSVIEW_OPENBSD)
static void OBSDGetIntrStats(std::vector<uint64_t> &intrCount,
  std::vector<unsigned int> &intrNbrs) {
    int nbr = 0;
    int nintr = 0;
    uint64_t count = 0;
    size_t size = sizeof(nintr);
    int mib_int[4] = { CTL_KERN, KERN_INTRCNT, KERN_INTRCNT_NUM };
    if ( sysctl(mib_int, 3, &nintr, &size, NULL, 0) < 0 ) {
        logProblem << "Could not get interrupt count" << std::endl;
        return;
    }
    for (int i = 0; i < nintr; i++) {
        mib_int[2] = KERN_INTRCNT_VECTOR;
        mib_int[3] = i;
        size = sizeof(nbr);
        if ( sysctl(mib_int, 4, &nbr, &size, NULL, 0) < 0 )
            continue;  // not active
        mib_int[2] = KERN_INTRCNT_CNT;
        size = sizeof(count);
        if ( sysctl(mib_int, 4, &count, &size, NULL, 0) < 0 ) {
            logProblem << "sysctl kern.intrcnt.cnt." << i << " failed\n";
            count = 0;
        }
        intrCount[nbr] += count;  // += because ints can share number
        intrNbrs[nbr] = 1;
    }
}
#endif


#if defined(XOSVIEW_NETBSD)
static void NBSDGetIntrStats(std::vector<uint64_t> &intrCount,
  std::vector<unsigned int> &intrNbrs) {

    const int Mib[] = { CTL_KERN, KERN_EVCNT, EVCNT_TYPE_INTR,
                        KERN_EVCNT_COUNT_ANY };
    static SysCtl evcnt_sc(Mib, sizeof(Mib) / sizeof(int));

    size_t evsize = 0;
    if (!evcnt_sc.getsize(evsize))
        logFatal << "sysctl(" << evcnt_sc.id() << ") failed." << std::endl;

    std::vector<char> buf(evsize, 0);
    if (!evcnt_sc.get(buf))
        logFatal << "sysctl(" << evcnt_sc.id() << ") failed." << std::endl;

    const struct evcnt_sysctl *evs =
        reinterpret_cast<const struct evcnt_sysctl *>(buf.data());
    const struct evcnt_sysctl *evsend =
        reinterpret_cast<const struct evcnt_sysctl *>(buf.data() + buf.size());

    while (evs->ev_len && evs < evsend &&
      reinterpret_cast<const struct evcnt_sysctl *>(
          (char *)evs + evs->ev_len * 8) < evsend ) {

        // extract the "pin" number from the name.
        std::string name(evs->ev_strings + evs->ev_grouplen + 1);
        size_t nbr = 0;
        std::string dummy;
        std::istringstream is(name);
        is >> dummy >> nbr;

        if (is) {
            intrCount[nbr] = evs->ev_count;
            intrNbrs[nbr] = 1;
        }

        evs = reinterpret_cast<const struct evcnt_sysctl *>((const char *)evs
          + 8 * evs->ev_len);
    }
}
#endif


void BSDGetIntrStats(std::vector<uint64_t> &intrCount,
  std::vector<unsigned int> &intrNbrs) {

    size_t intVectorLen = BSDNumInts() + 1;
    intrCount.resize(intVectorLen);
    intrNbrs.resize(intVectorLen);

#if defined(XOSVIEW_FREEBSD)
    FBSDGetIntrStats(intrCount, intrNbrs);
#elif defined(XOSVIEW_DFBSD)
    DFBSDGetIntrStats(intrCount, intrNbrs);
#elif defined(XOSVIEW_NETBSD)
    NBSDGetIntrStats(intrCount, intrNbrs);
#elif defined(XOSVIEW_OPENBSD)
    OBSDGetIntrStats(intrCount, intrNbrs);
#endif
}


//  ---------------------- Sensor Meter stuff  ---------------------------------

#if defined(__i386__) || defined(__x86_64__)
static unsigned int BSDGetCPUTemperatureMap(std::map<int, float> &temps,
  std::map<int, float> &tjmax) {
    temps.clear();
    tjmax.clear();
    unsigned int nbr = 0;
#if defined(XOSVIEW_OPENBSD) || defined(XOSVIEW_DFBSD)
    (void)tjmax; // Avoid the warning
#endif
#if defined(XOSVIEW_NETBSD)
    // All kinds of sensors are read with libprop. We have to go through them
    // to find either Intel Core 2 or AMD ones. Actual temperature is in
    // cur-value and TjMax, if present, in critical-max.
    // Values are in microdegrees Kelvin.
    int fd;
    const char *name = NULL;
    prop_dictionary_t pdict;
    prop_object_t pobj, pobj1, pobj2;
    prop_object_iterator_t piter, piter2;
    prop_array_t parray;

    if ( (fd = open(_PATH_SYSMON, O_RDONLY)) == -1 ) {
        logProblem << "Could not open " << _PATH_SYSMON << std::endl;
        return 0;  // this seems to happen occasionally, so only warn
    }
    if (prop_dictionary_recv_ioctl(fd, ENVSYS_GETDICTIONARY, &pdict))
        logFatal << "Could not get sensor dictionary" << std::endl;
    if (close(fd) == -1)
        logFatal << "Could not close " << _PATH_SYSMON << std::endl;

    if (prop_dictionary_count(pdict) == 0) {
        logProblem << "No sensors found" << std::endl;
        return 0;
    }
    if ( !(piter = prop_dictionary_iterator(pdict)) )
        logFatal << "Could not get sensor iterator" << std::endl;

    while ( (pobj = prop_object_iterator_next(piter)) ) {
        parray = (prop_array_t)prop_dictionary_get_keysym(pdict,
          (prop_dictionary_keysym_t)pobj);
        if (prop_object_type(parray) != PROP_TYPE_ARRAY)
            continue;
        name = prop_dictionary_keysym_cstring_nocopy(
            (prop_dictionary_keysym_t)pobj);
        if (std::string(name, 0, 8) != "coretemp"
          && std::string(name, 0, 7) != "amdtemp")
            continue;
        if ( !(piter2 = prop_array_iterator(parray)) )
            logFatal << "Could not get sensor iterator" << std::endl;

        int i = 0;
        std::istringstream is(name);
        is >> util::sink("*[!0-9]", true) >> i;
        while ( (pobj = prop_object_iterator_next(piter2)) ) {
            if ( !(pobj1 = prop_dictionary_get((prop_dictionary_t)pobj,
                  "type")) )
                continue;
            if ( (pobj1 = prop_dictionary_get((prop_dictionary_t)pobj,
                  "cur-value")) ) {
                temps[i] = (prop_number_integer_value((prop_number_t)pobj1)
                  / 1000000.0) - 273.15;
                nbr++;
            }
            if ( (pobj2 = prop_dictionary_get((prop_dictionary_t)pobj,
                  "critical-max")))
                tjmax[i] = (prop_number_integer_value(
                      (prop_number_t)pobj2) / 1000000.0) - 273.15;
        }
        prop_object_iterator_release(piter2);
    }
    prop_object_iterator_release(piter);
    prop_object_release(pdict);
#else  /* XOSVIEW_NETBSD */
    int val = 0;
    size_t size = sizeof(val);

#if defined(XOSVIEW_OPENBSD) || defined(XOSVIEW_DFBSD)
    // All kinds of sensors are read with sysctl. We have to go through them
    // to find either Intel Core 2 or AMD ones.
    // Values are in microdegrees Kelvin.
    struct sensordev sd;
    struct sensor s;
    int cpu = 0;
    int mib_sen[] = { CTL_HW, HW_SENSORS, 0, 0, 0 };

    for (int dev = 0; dev < 1024; dev++) {  // go through all sensor devices
        mib_sen[2] = dev;
        size = sizeof(sd);
        if ( sysctl(mib_sen, 3, &sd, &size, NULL, 0) < 0 ) {
            if (errno == ENOENT)
                break;  // no more sensors
            if (errno == ENXIO)
                continue;  // no sensor with this mib
            logFatal << "sysctl hw.sensors." << dev << " failed" << std::endl;
        }
        if (std::string(sd.xname, 0, 3) != "cpu")
            continue;  // not CPU sensor
        std::istringstream is(sd.xname);
        is >> util::sink("*[!0-9]", true) >> cpu;

        mib_sen[3] = SENSOR_TEMP;  // for each device, get temperature sensors
        for (int i = 0; i < sd.maxnumt[SENSOR_TEMP]; i++) {
            mib_sen[4] = i;
            size = sizeof(s);
            if ( sysctl(mib_sen, 5, &s, &size, NULL, 0) < 0 )
                continue;  // no sensor on this core?
            if (s.flags & SENSOR_FINVALID)
                continue;
            temps[cpu] = (float)(s.value - 273150000) / 1000000.0;
            nbr++;
        }
    }
#else  /* XOSVIEW_FREEBSD */
    // Temperatures can be read with sysctl dev.cpu.%d.temperature on both
    // Intel Core 2 and AMD K8+ processors.
    // Values are in degrees Celsius (FreeBSD < 7.2) or in
    // 10*degrees Kelvin (FreeBSD >= 7.3).
    std::string name;
    int cpus = BSDCountCpus();
    for (int i = 0; i < cpus; i++) {
        name = "dev.cpu." + util::repr(i) + ".temperature";
        if ( sysctlbyname(name.c_str(), &val, &size, NULL, 0) == 0) {
            nbr++;
#if __FreeBSD_version >= 702106
            temps[i] = ((float)val - 2732.0) / 10.0;
#else
            temps[i] = (float)val;
#endif
        }
        else
            logProblem << "sysctl " << name << " failed" << std::endl;

        name = "dev.cpu." + util::repr(i) + ".coretemp.tjmax";
        if ( sysctlbyname(name.c_str(), &val, &size, NULL, 0) == 0 )
#if __FreeBSD_version >= 702106
            tjmax[i] = ((float)val - 2732.0) / 10.0;
#else
            tjmax[i] = (float)val;
#endif
        else
            logProblem << "sysctl " << name << " failed\n";
    }
#endif
#endif
    return nbr;
}


unsigned int BSDGetCPUTemperature(std::vector<float> &temps,
  std::vector<float> &tjmax) {
    std::map<int, float> tempM;
    std::map<int, float> tjmxM;
    unsigned int count = BSDGetCPUTemperatureMap(tempM, tjmxM);
    if (tempM.size() != count || tjmxM.size() != count)
        logFatal << "Internal core temp logic failure." << std::endl;

    temps.resize(count);
    tjmax.resize(count);

    // The std::map version that gets the data reads the cpu number
    // from a string.  The assumption is that this cpu number will
    // somehow be in the range [0-count].  We will populate the
    // vectors here and check this.  If it is not the case then
    // the train stops here.
    for (std::map<int,float>::iterator it=tempM.begin(); it!=tempM.end(); ++it)
        if (it->first >= 0 && it->first < (int)temps.size())
            temps[it->first] = it->second;
        else
            logFatal << "Internal core temp logic failure." << std::endl;

    for (std::map<int,float>::iterator it=tjmxM.begin(); it!=tjmxM.end(); ++it)
        if (it->first >= 0 && it->first < (int)temps.size())
            tjmax[it->first] = it->second;
        else
            logFatal << "Internal core temp logic failure." << std::endl;

    return count;
}

#endif


void BSDGetSensor(const std::string &name, const std::string &valname,
  float &value, std::string &unit) {

    logAssert(name.size() && valname.size()
      && value) << "'NULL' pointer passed to BSDGetSensor()." << std::endl;

#if defined(XOSVIEW_NETBSD)
    /* Adapted from envstat. */
    // All kinds of sensors are read with libprop. Specific device and value
    // can be asked for. Values are transformed to suitable units.
    int fd, val = 0;
    prop_dictionary_t pdict;
    prop_object_t pobj, pobj1;
    prop_object_iterator_t piter;

    if ( (fd = open(_PATH_SYSMON, O_RDONLY)) == -1 ) {
        logProblem << "Could not open " << _PATH_SYSMON << std::endl;
        return;  // this seems to happen occasionally, so only warn
    }
    if (prop_dictionary_recv_ioctl(fd, ENVSYS_GETDICTIONARY, &pdict))
        logFatal << "Could not get sensor dictionary" << std::endl;
    if (close(fd) == -1)
        logFatal << "Could not close " << _PATH_SYSMON << std::endl;

    if (prop_dictionary_count(pdict) == 0) {
        logProblem << "No sensors found" << std::endl;
        return;
    }
    pobj = prop_dictionary_get(pdict, name.c_str());
    if (prop_object_type(pobj) != PROP_TYPE_ARRAY)
        logFatal << "Device " << name << " does not exist" << std::endl;

    if ( !(piter = prop_array_iterator((prop_array_t)pobj)) )
        logFatal << "Could not get sensor iterator" << std::endl;

    while ( (pobj = prop_object_iterator_next(piter)) ) {
        if ( !(pobj1 = prop_dictionary_get((prop_dictionary_t)pobj, "type")) )
            continue;

        std::string ptype(prop_string_cstring_nocopy((prop_string_t)pobj1));

        if ( ptype == "Indicator" || ptype == "Battery"
          || ptype == "Drive" )
            continue;  // these are string values
        if ( (pobj1 = prop_dictionary_get((prop_dictionary_t)pobj,
              valname.c_str())) )
            val = prop_number_integer_value((prop_number_t)pobj1);
        else
            logFatal << "Value " << valname << " does not exist\n";
        if ( ptype == "Temperature" ) {
            value = (val / 1000000.0) - 273.15;  // temps are in microkelvins
            unit = "\260C";
        }
        else if ( ptype == "Fan") {
            value = (float)val;                  // plain integer value
            unit = "RPM";
        }
        else if ( ptype == "Integer" )
            value = (float)val;                  // plain integer value
        else if ( ptype == "Voltage" ) {
            value = (float)val / 1000000.0;      // units are micro{V,A,W,Ohm}
            unit = "V";
        }
        else if ( ptype == "Ampere hour" ) {
            value = (float)val / 1000000.0;      // units are micro{V,A,W,Ohm}
            unit = "Ah";
        }
        else if ( ptype == "Ampere" ) {
            value = (float)val / 1000000.0;      // units are micro{V,A,W,Ohm}
            unit = "A";
        }
        else if ( ptype == "Watt hour" ) {
            value = (float)val / 1000000.0;      // units are micro{V,A,W,Ohm}
            unit = "Wh";
        }
        else if ( ptype == "Watts" ) {
            value = (float)val / 1000000.0;      // units are micro{V,A,W,Ohm}
            unit = "W";
        }
        else if ( ptype == "Ohms" ) {
            value = (float)val / 1000000.0;      // units are micro{V,A,W,Ohm}
            unit = "Ohm";
        }
    }
    prop_object_iterator_release(piter);
    prop_object_release(pdict);
#else  /* XOSVIEW_NETBSD */
    size_t size;
    std::string dummy;
#if defined(XOSVIEW_FREEBSD) || defined(XOSVIEW_DFBSD)
    // FreeBSD has no sensor framework, but ACPI thermal zones might work.
    // They are readable through sysctl (also works in Dragonfly).
    // Values are in 10 * degrees Kelvin.
    if ( name == "tz" ) {
        int val = 0;
        size = sizeof(val);
        dummy = "hw.acpi.thermal." + name + "." + valname;
        if ( sysctlbyname(dummy.c_str(), &val, &size, NULL, 0) < 0 )
            logFatal << "sysctl " << dummy << " failed" << std::endl;
        value = ((float)val - 2732.0) / 10.0;
        if (unit.size())
            unit = "\260C";
        return;
    }
    // If Dragonfly and tzN specified, return. Otherwise, fall through.
#endif
#if defined(XOSVIEW_OPENBSD) || defined(XOSVIEW_DFBSD)
    /* Adapted from systat. */
    // All kinds of sensors are read with sysctl. We have to go through them
    // to find the required device and value. Parameter 'name' is the device
    // name and 'valname' consists of type and sensor index (e.g. it0.temp1).
    //  Values are transformed to suitable units.
    int index = -1;
    struct sensordev sd;
    struct sensor s;
    int mib_sen[] = { CTL_HW, HW_SENSORS, 0, 0, 0 };

    for (int dev = 0; dev < 1024; dev++) {  // go through all sensor devices
        mib_sen[2] = dev;
        size = sizeof(sd);
        if ( sysctl(mib_sen, 3, &sd, &size, NULL, 0) < 0 ) {
            if (errno == ENOENT)
                break;  // no more devices
            if (errno == ENXIO)
                continue;  // no device with this mib
            logFatal << "sysctl hw.sensors." << dev << " failed" << std::endl;
        }
        std::string sname(name);
        if (std::string(sd.xname).substr(0, sname.size()) == name)
            continue;  // sensor name does not match

        for (int t = 0; t < SENSOR_MAX_TYPES; t++) {
            std::string stype_s(sensor_type_s[t]);
            if (stype_s != valname.substr(0, stype_s.size()))
                continue;  // wrong type
            mib_sen[3] = t;
            std::istringstream is(valname);
            is >> util::sink("*[!0-9]", true) >> index;
            if (index < sd.maxnumt[t]) {
                mib_sen[4] = index;
                size = sizeof(s);
                if ( sysctl(mib_sen, 5, &s, &size, NULL, 0) < 0 ) {
                    if (errno != ENOENT)
                        logFatal << "sysctl hw.sensors."
                                 << dev << "." << t << "." << index
                                 << " failed" << std::endl;

                    continue;  // no more sensors
                }
                if (s.flags & SENSOR_FINVALID)
                    continue;
                switch (t) {
                case SENSOR_TEMP:
                    value = (float)(s.value - 273150000) / 1000000.0;
                    unit = "\260C";
                    break;
                case SENSOR_FANRPM:
                    value = (float)s.value;
                    unit = "RPM";
                    break;
                case SENSOR_VOLTS_DC:
                case SENSOR_VOLTS_AC:
                    value = (float)s.value / 1000000.0;
                    unit = "V";
                    break;
                case SENSOR_OHMS:
                    value = (float)s.value;
                    unit = "Ohm";
                    break;
                case SENSOR_WATTS:
                    value = (float)s.value / 1000000.0;
                    unit = "W";
                    break;
                case SENSOR_AMPS:
                    value = (float)s.value / 1000000.0;
                    unit = "A";
                    break;
                case SENSOR_WATTHOUR:
                    value = (float)s.value / 1000000.0;
                    unit = "Wh";
                    break;
                case SENSOR_AMPHOUR:
                    value = (float)s.value / 1000000.0;
                    unit = "Ah";
                    break;
                case SENSOR_PERCENT:
                    value = (float)s.value / 1000.0;
                    unit = "%";
                    break;
                case SENSOR_LUX:
                    value = (float)s.value / 1000000.0;
                    unit = "lx";
                    break;
                case SENSOR_TIMEDELTA:
                    value = (float)s.value / 1000000000.0;
                    unit = "s";
                    break;
#if defined(XOSVIEW_OPENBSD)
                case SENSOR_HUMIDITY:
                    value = (float)s.value / 1000.0;
                    unit = "%";
                    break;
                case SENSOR_FREQ:
                    value = (float)s.value / 1000000.0;
                    unit = "Hz";
                    break;
                case SENSOR_ANGLE:
                    value = (float)s.value / 1000000.0;
                    unit = "\260";
                    break;
#if OpenBSD > 201211
                case SENSOR_DISTANCE:
                    value = (float)s.value / 1000000.0;
                    unit = "m";
                    break;
                case SENSOR_PRESSURE:
                    value = (float)s.value / 1000.0;
                    unit = "Pa";
                    break;
                case SENSOR_ACCEL:
                    value = (float)s.value / 1000000.0;
                    unit = "m\\/s\262"; // m/sÅ¬Å≤
                    break;
#endif
#endif
                case SENSOR_INDICATOR:
                case SENSOR_INTEGER:
                case SENSOR_DRIVE:
                default:
                    value = (float)s.value;
                    break;
                }
            }
        }
    }
#endif
#endif
}


//  ---------------------- Battery Meter stuff ---------------------------------

bool BSDHasBattery() {
#if defined(XOSVIEW_NETBSD)
    int fd;
    prop_dictionary_t pdict;
    prop_object_t pobj;

    if ( (fd = open(_PATH_SYSMON, O_RDONLY)) == -1 )
        return false;
    if ( prop_dictionary_recv_ioctl(fd, ENVSYS_GETDICTIONARY, &pdict) )
        logFatal << "Could not get sensor dictionary" << std::endl;
    if ( close(fd) == -1 )
        logFatal << "Could not close " << _PATH_SYSMON << std::endl;

    if ( prop_dictionary_count(pdict) == 0 )
        return false;
    pobj = prop_dictionary_get(pdict, "acpibat0"); // just check for 1st battery
    if ( prop_object_type(pobj) != PROP_TYPE_ARRAY )
        return false;
    return true;
#elif defined(XOSVIEW_OPENBSD)
    // check if we can get full capacity of the 1st battery
    float val = -1.0;
    std::string emptyStr;
    BSDGetSensor("acpibat0", "amphour0", val, emptyStr);
    if (val < 0)
        return false;
    return true;
#else // XOSVIEW_FREEBSD || XOSVIEW_DFBSD
    int fd;
    if ( (fd = open(ACPIDEV, O_RDONLY)) == -1 ) {
        // No ACPI -> try APM
        if ( (fd = open(APMDEV, O_RDONLY)) == -1 )
            return false;
        struct apm_info aip;
        if ( ioctl(fd, APMIO_GETINFO, &aip) == -1 )
            return false;
        if ( close(fd) == -1 )
            logFatal << "Could not close " << APMDEV << std::endl;
        if (aip.ai_batt_stat == 0xff || aip.ai_batt_life == 0xff)
            return false;
        return true;
    }

    union acpi_battery_ioctl_arg battio;
    battio.unit = ACPI_BATTERY_ALL_UNITS;
    if ( ioctl(fd, ACPIIO_BATT_GET_BATTINFO, &battio) == -1 )
        return false;
    if ( close(fd) == -1 )
        logFatal << "Could not close " << ACPIDEV << std::endl;
    return ( battio.battinfo.state != ACPI_BATT_STAT_NOT_PRESENT );
#endif
}


void BSDGetBatteryInfo(int &remaining, unsigned int &state) {
    state = XOSVIEW_BATT_NONE;
#if defined(XOSVIEW_NETBSD) || defined(XOSVIEW_OPENBSD)
    int batteries = 0;
#if defined(XOSVIEW_NETBSD)
    /* Again adapted from envstat. */
    // All kinds of sensors are read with libprop. We have to go through them
    // to find the batteries. We need capacity, charge, presence, charging
    // status and discharge rate for each battery for the calculations.
    // For simplicity, assume all batteries have the same
    // charge/discharge status.
    int fd;
    int total_capacity = 0, total_charge = 0, total_low = 0, total_crit = 0;
    const char *name = NULL;
    prop_dictionary_t pdict;
    prop_object_t pobj, pobj1;
    prop_object_iterator_t piter, piter2;
    prop_array_t parray;

    if ( (fd = open(_PATH_SYSMON, O_RDONLY)) == -1 ) {
        logProblem << "Could not open " << _PATH_SYSMON << std::endl;
        return;  // this seems to happen occasionally, so only warn
    }
    if ( prop_dictionary_recv_ioctl(fd, ENVSYS_GETDICTIONARY, &pdict) )
        logFatal << "Could not get sensor dictionary" << std::endl;
    if ( close(fd) == -1 )
        logFatal << "Could not close " << _PATH_SYSMON << std::endl;

    if ( prop_dictionary_count(pdict) == 0 ) {
        logProblem << "No sensors found" << std::endl;
        return;
    }
    if ( !(piter = prop_dictionary_iterator(pdict)) )
        logFatal << "Could not get sensor iterator" << std::endl;

    while ( (pobj = prop_object_iterator_next(piter)) ) {
        int present = 0, capacity = 0, charge = 0, low = 0, crit = 0;
        name = prop_dictionary_keysym_cstring_nocopy(
            (prop_dictionary_keysym_t)pobj);
        if ( std::string(name, 0, 7) != "acpibat" )
            continue;
        parray = (prop_array_t)prop_dictionary_get_keysym(pdict,
          (prop_dictionary_keysym_t)pobj);
        if ( prop_object_type(parray) != PROP_TYPE_ARRAY )
            continue;
        if ( !(piter2 = prop_array_iterator(parray)) )
            logFatal << "Could not get sensor iterator" << std::endl;

        while ( (pobj = prop_object_iterator_next(piter2)) ) {
            if ( !(pobj1 = prop_dictionary_get((prop_dictionary_t)pobj,
                  "state")) )
                continue;
            if ( prop_string_equals_cstring((prop_string_t)pobj1, "invalid")
              || prop_string_equals_cstring((prop_string_t)pobj1, "unknown") )
                continue; // skip sensors without valid data
            if ( !(pobj1 = prop_dictionary_get((prop_dictionary_t)pobj,
                  "description")) )
                continue;
            name = prop_string_cstring_nocopy((prop_string_t)pobj1);
            if ( std::string(name, 0, 7) == "present" ) { // is battery present
                if ( (pobj1 = prop_dictionary_get((prop_dictionary_t)pobj,
                      "cur-value")) )
                    present = prop_number_integer_value((prop_number_t)pobj1);
            }
            else if ( std::string(name, 0, 10) == "design cap" ) {
                // get full capacity
                if ( (pobj1 = prop_dictionary_get((prop_dictionary_t)pobj,
                      "cur-value")) )
                    capacity = prop_number_integer_value((prop_number_t)pobj1);
            }
            else if ( std::string(name, 0, 7) == "charge" ) {
                // get present charge, low and critical levels
                if ( (pobj1 = prop_dictionary_get((prop_dictionary_t)pobj,
                      "cur-value")) )
                    charge = prop_number_integer_value((prop_number_t)pobj1);
                if ( (pobj1 = prop_dictionary_get((prop_dictionary_t)pobj,
                      "warning-capacity")) )
                    low = prop_number_integer_value((prop_number_t)pobj1);
                if ( (pobj1 = prop_dictionary_get((prop_dictionary_t)pobj,
                      "critical-capacity")) )
                    crit = prop_number_integer_value((prop_number_t)pobj1);
            }
            else if ( std::string(name, 0, 8) == "charging" ) {
                // charging or not?
                if ( (pobj1 = prop_dictionary_get((prop_dictionary_t)pobj,
                      "cur-value")) )
                    if ( prop_number_integer_value((prop_number_t)pobj1) )
                        state |= XOSVIEW_BATT_CHARGING;
            }
            else if (std::string(name, 0, 14) == "discharge rate") {
                // discharging or not?
                if ( (pobj1 = prop_dictionary_get((prop_dictionary_t)pobj,
                      "cur-value")) )
                    if ( prop_number_integer_value((prop_number_t)pobj1) )
                        state |= XOSVIEW_BATT_DISCHARGING;
            }
        }
        if (present) {
            total_capacity += capacity;
            total_charge += charge;
            total_low += low;
            total_crit += crit;
            batteries++;
        }
        prop_object_iterator_release(piter2);
    }
    prop_object_iterator_release(piter);
    prop_object_release(pdict);
#else // XOSVIEW_OPENBSD
    float total_capacity = 0, total_charge = 0, total_low = 0, total_crit = 0;
    std::string battery;
    while (batteries < 1024) {
        float val = -1.0;
        battery = "acpibat" + util::repr(batteries);
        std::string emptyStr;
        BSDGetSensor(battery, "amphour0", val, emptyStr); // full capacity
        if (val < 0) // no more batteries
            break;
        batteries++;
        total_capacity += val;
        emptyStr = "";
        BSDGetSensor(battery, "amphour1", val, emptyStr); // warning capacity
        total_low += val;
        emptyStr = "";
        BSDGetSensor(battery, "amphour2", val, emptyStr); // low capacity
        total_crit += val;
        emptyStr = "";
        BSDGetSensor(battery, "amphour3", val, emptyStr); // remaining
        total_charge += val;
        emptyStr = "";
        BSDGetSensor(battery, "raw0", val, emptyStr); // state
        if ((int)val == 1)
            state |= XOSVIEW_BATT_DISCHARGING;
        else if ((int)val == 2)
            state |= XOSVIEW_BATT_CHARGING;
        // there's also 0 state for idle/full
    }
#endif
    if (batteries == 0) { // all batteries are off
        state = XOSVIEW_BATT_NONE;
        remaining = 0;
        return;
    }
    remaining = 100 * total_charge / total_capacity;
    if ( !(state & XOSVIEW_BATT_CHARGING) &&
      !(state & XOSVIEW_BATT_DISCHARGING) )
        state |= XOSVIEW_BATT_FULL;  // full when not charging nor discharging
    if (total_capacity < total_low)
        state |= XOSVIEW_BATT_LOW;
    if (total_capacity < total_crit)
        state |= XOSVIEW_BATT_CRITICAL;
#else // XOSVIEW_FREEBSD || XOSVIEW_DFBSD
    /* Adapted from acpiconf and apm. */
    int fd;
    if ( (fd = open(ACPIDEV, O_RDONLY)) == -1 ) {
        // No ACPI -> try APM
        if ( (fd = open(APMDEV, O_RDONLY)) == -1 )
            logFatal << "could not open " << ACPIDEV << " or " << APMDEV
                     << std::endl;

        struct apm_info aip;
        if ( ioctl(fd, APMIO_GETINFO, &aip) == -1 )
            logFatal << "failed to get APM battery info" << std::endl;
        if ( close(fd) == -1 )
            logFatal << "Could not close " << APMDEV << std::endl;
        if (aip.ai_batt_life <= 100)
            remaining = aip.ai_batt_life; // only 0-100 are valid values
        else
            remaining = 0;
        if (aip.ai_batt_stat == 0)
            state |= XOSVIEW_BATT_FULL;
        else if (aip.ai_batt_stat == 1)
            state |= XOSVIEW_BATT_LOW;
        else if (aip.ai_batt_stat == 2)
            state |= XOSVIEW_BATT_CRITICAL;
        else if (aip.ai_batt_stat == 3)
            state |= XOSVIEW_BATT_CHARGING;
        else
            state = XOSVIEW_BATT_NONE;
        return;
    }
    // ACPI
    union acpi_battery_ioctl_arg battio;
    battio.unit = ACPI_BATTERY_ALL_UNITS;
    if ( ioctl(fd, ACPIIO_BATT_GET_BATTINFO, &battio) == -1 )
        logFatal << "failed to get ACPI battery info" << std::endl;
    if ( close(fd) == -1 )
        logFatal << "Could not close " << ACPIDEV << std::endl;
    remaining = battio.battinfo.cap;
    if (battio.battinfo.state != ACPI_BATT_STAT_NOT_PRESENT) {
        if (battio.battinfo.state == 0)
            state |= XOSVIEW_BATT_FULL;
        if (battio.battinfo.state & ACPI_BATT_STAT_CRITICAL)
            state |= XOSVIEW_BATT_CRITICAL;
        if (battio.battinfo.state & ACPI_BATT_STAT_DISCHARG)
            state |= XOSVIEW_BATT_DISCHARGING;
        if (battio.battinfo.state & ACPI_BATT_STAT_CHARGING)
            state |= XOSVIEW_BATT_CHARGING;
    }
#endif
}
