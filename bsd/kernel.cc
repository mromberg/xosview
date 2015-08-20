//
//  NetBSD port:
//  Copyright (c) 1995, 1996, 1997-2002, 2015
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

#include "log.h"
#include "strutil.h"
#include "kernel.h"

#include <unistd.h>
#include <fcntl.h>
#include <kvm.h>
#include <nlist.h>
#include <limits.h>
#include <errno.h>
#include <sysexits.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <net/if.h>

#if defined(XOSVIEW_DFBSD)
#define _KERNEL_STRUCTURES
#include <kinfo.h>
#endif

#if defined(XOSVIEW_FREEBSD) || defined(XOSVIEW_DFBSD)
static const char ACPIDEV[] = "/dev/acpi";
static const char APMDEV[] = "/dev/apm";
static int maxcpus = 1;
#include <net/if_var.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <dev/acpica/acpiio.h>
#include <machine/apm_bios.h>
#endif

#if defined(XOSVIEW_NETBSD)
#include <sys/sched.h>
#include <sys/iostat.h>
#include <sys/envsys.h>
#include <prop/proplib.h>
#include <paths.h>
static int mib_cpt[2] = { CTL_KERN, KERN_CP_TIME };
static int mib_dsk[3] = { CTL_HW, HW_IOSTATS, sizeof(struct io_sysctl) };
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
static int mib_spd[2] = { CTL_HW, HW_CPUSPEED };
static int mib_cpt[2] = { CTL_KERN, KERN_CPTIME };
static int mib_cpt2[3] = { CTL_KERN, KERN_CPTIME2, 0 };
static int mib_ifl[6] = { CTL_NET, AF_ROUTE, 0, 0, NET_RT_IFLIST, 0 };
#endif

#if defined(XOSVIEW_OPENBSD) || defined(XOSVIEW_DFBSD)
#include <sys/sensors.h>
static int mib_sen[5] = { CTL_HW, HW_SENSORS };
#endif

#if defined(HAVE_DEVSTAT)
#include <devstat.h>
#endif

#if defined(HAVE_UVM)
#include <string.h>
#include <sys/malloc.h>
#include <sys/device.h>
#include <uvm/uvm_extern.h>
#ifdef VM_UVMEXP2
static int mib_uvm[2] = { CTL_VM, VM_UVMEXP2 };
#else
static int mib_uvm[2] = { CTL_VM, VM_UVMEXP };
#endif
#else
#include <sys/vmmeter.h>
#endif

#if defined(HAVE_SWAPCTL)
#include <sys/swap.h>
#endif


// ------------------------  local variables  ----------------------------------

//  This single kvm_t is shared by all of the kvm routines.
kvm_t* kd = NULL;

//  This struct has the list of all the symbols we want from the kernel.
static struct nlist nlst[] = {
// We put a dummy symbol for a don't care, and ignore warnings about
// this later on.  This keeps the indices within the nlist constant.
#define DUMMY_SYM "dummy_sym"

#if defined(XOSVIEW_FREEBSD)
    { "_cnt" },
#define VMMETER_SYM_INDEX    0
#else
    { DUMMY_SYM },
#define DUMMY_0
#endif
#if !defined(XOSVIEW_OPENBSD)
    { "_ifnet" },
#define IFNET_SYM_INDEX      1
#else
    { DUMMY_SYM },
#define DUMMY_1
#endif

#if defined(XOSVIEW_OPENBSD)
    { "_disklist" },
#define DISKLIST_SYM_INDEX   2
#else
    { DUMMY_SYM },
#define DUMMY_2
#endif
#if defined(XOSVIEW_NETBSD)
    { "_allevents" },
#define ALLEVENTS_SYM_INDEX  3
    { "_bufmem" },
#define BUFMEM_SYM_INDEX     4
#else
    { DUMMY_SYM },
#define DUMMY_3
    { DUMMY_SYM },
#define DUMMY_4
#endif
#if defined(XOSVIEW_FREEBSD)
    { "_intrnames" },
#define INTRNAMES_SYM_INDEX  5
# if __FreeBSD_version >= 900040
    { "_sintrnames" },
# else
    { "_eintrnames" },
# endif
#define EINTRNAMES_SYM_INDEX 6
    { "_intrcnt" },
#define INTRCNT_SYM_INDEX    7
# if __FreeBSD_version >= 900040
    { "_sintrcnt" },
# else
    { "_eintrcnt" },
# endif
#define EINTRCNT_SYM_INDEX   8
#endif
    { NULL }
};

static char kernelFileName[_POSIX2_LINE_MAX];


// ------------------------  utility functions  --------------------------------
//  The following is an error-checking form of kvm_read.  In addition
//  it uses kd as the implicit kernel-file to read.  Saves typing.
//  Since this is C++, it's an inline function rather than a macro.

static inline void safe_kvm_read(unsigned long kernel_addr, void* user_addr,
  size_t nbytes) {

    /*  Check for obvious bad symbols (i.e., from /netbsd when we
     *  booted off of /netbsd.old), such as symbols that reference
     *  0x00000000 (or anywhere in the first 256 bytes of memory).  */
    int retval = 0;

    if ( (kernel_addr & 0xffffff00) == 0 )
        logFatal << "safe_kvm_read() was attempted on EA "
                 << std::hex << std::showbase << kernel_addr << std::endl;

    if ( (retval = kvm_read(kd, kernel_addr, user_addr, nbytes)) == -1 )
        logFatal << "kvm_read() of kernel address "
                 << std::hex << std::showbase << kernel_addr << std::endl;

    logAssert(retval == (int)nbytes) << "safe_kvm_read("
                                     << std::hex << std::showbase
                                     << kernel_addr << ") "
                                     << "returned " << std::dec
                                     << retval << " bytes, "
                                     << "not " << (int)nbytes << std::endl;
}


//  This version uses the symbol offset in the nlst variable, to make it
//  a little more convenient.  BCG
static inline void safe_kvm_read_symbol(int nlstOffset, void* user_addr,
  size_t nbytes) {

    safe_kvm_read(nlst[nlstOffset].n_value, user_addr, nbytes);
}


int ValidSymbol(int index) {
    return ( (nlst[index].n_value & 0xffffff00) != 0 );
}


int SymbolValue(int index) {
    return nlst[index].n_value;
}


void BSDInit() {
    kernelFileName[0] = '\0';
}


void SetKernelName(const std::string &kernelName) {

    if (kernelName.size() >= _POSIX2_LINE_MAX)
        logFatal << "Kernel file name of '"
                 << kernelName << "' is too long." << std::endl;

    kernelName.copy(kernelFileName, _POSIX2_LINE_MAX);
}


void OpenKDIfNeeded() {

    char errstring[_POSIX2_LINE_MAX];

    if (kd)
        return; //  kd is non-NULL, so it has been initialized.  BCG

    /*  Open it read-only, for a little added safety.  */
    /*  If the first character of kernelFileName is not '\0', then use
     *  that kernel file.  Otherwise, use the default kernel, by
     *  specifying NULL.
     */
    if ((kd = kvm_openfiles((kernelFileName[0] ? kernelFileName : NULL),
          NULL, NULL, O_RDONLY, errstring)) == NULL)
        logFatal << "OpenKDIfNeeded(): " << errstring << std::endl;

    // Parenthetical note:  FreeBSD kvm_openfiles() uses getbootfile() to get
    // the correct kernel file if the 1st arg is NULL.  As far as I can see,
    // one should always use NULL in FreeBSD, but I suppose control is never a
    // bad thing... (pavel 21-Jan-1998)

    /*  Now grab the symbol offsets for the symbols that we want.  */
    if (kvm_nlist(kd, nlst) < 0)
        logFatal << "Could not get kvm symbols" << std::endl;

    //  Look at all of the returned symbols, and check for bad lookups.
    //  (This may be unnecessary, but better to check than not to...  )
    struct nlist *nlp = nlst;
    std::string sDUMMY_SYM(DUMMY_SYM);
    while (nlp && nlp->n_name) {
        if (std::string(nlp->n_name, 0, sDUMMY_SYM.size()) != sDUMMY_SYM) {
            if ( nlp->n_type == 0 || nlp->n_value == 0 )
#if defined(XOSVIEW_FREEBSD) && defined(__alpha__)
                /* XXX: this should be properly fixed. */
                ;
#else
            logProblem << "kvm_nlist() lookup failed for symbol '"
                       << nlp->n_name << "'" << std::endl;
#endif
        }
        nlp++;
    }
}


int BSDGetCPUSpeed() {
    size_t size;
    int cpu_speed = 0;

#if defined(XOSVIEW_FREEBSD)
    std::string name;
    int speed = 0, cpus = BSDCountCpus(), avail_cpus = 0;
    size = sizeof(speed);
    for (int i = 0; i < cpus; i++) {
        name = "dev.cpu." + util::repr(i) + ".freq";
        if ( sysctlbyname(name.c_str(), &speed, &size, NULL, 0) == 0 ) {
            // count only cpus with individual freq available
            cpu_speed += speed;
            avail_cpus++;
        }
    }
    if (avail_cpus > 1)
        cpu_speed /= avail_cpus;
#elif defined(XOSVIEW_OPENBSD)
    size = sizeof(cpu_speed);
    if ( sysctl(mib_spd, 2, &cpu_speed, &size, NULL, 0) < 0 )
        logFatal << "syscl hw.cpuspeed failed" << std::endl;
#else  /* XOSVIEW_NETBSD || XOSVIEW_DFBSD */
    uint64_t speed = 0;
    size = sizeof(speed);
#if defined(XOSVIEW_NETBSD)
    if ( sysctlbyname("machdep.tsc_freq", &speed, &size, NULL, 0) < 0 )
        logFatal << "sysctl machdep.tsc_freq failed" << std::endl;
#else  /* XOSVIEW_DFBSD */
    if ( sysctlbyname("hw.tsc_frequency", &speed, &size, NULL, 0) < 0 )
        logFatal << "sysctl hw.tsc_frequency failed" << std::endl;
#endif
    cpu_speed = speed / 1000000;
#endif
    return cpu_speed;
}


// --------------------  PageMeter & MemMeter functions  -----------------------
void BSDPageInit() {
    OpenKDIfNeeded();
}


/* meminfo[5]  = { active, inactive, wired, cached, free } */
/* pageinfo[2] = { pages_in, pages_out }                   */
void BSDGetPageStats(uint64_t *meminfo, uint64_t *pageinfo) {
#if defined(HAVE_UVM)
#ifdef VM_UVMEXP2
    struct uvmexp_sysctl uvm;
#else
    struct uvmexp uvm;
#endif
    size_t size = sizeof(uvm);
    if ( sysctl(mib_uvm, 2, &uvm, &size, NULL, 0) < 0 )
        logFatal << "sysctl vm.uvmexp failed" << std::endl;

    if (meminfo) {
        // UVM excludes kernel memory -> assume it is active mem
        meminfo[0] = (uint64_t)(uvm.npages - uvm.inactive - uvm.wired
          - uvm.free) * uvm.pagesize;
        meminfo[1] = (uint64_t)uvm.inactive * uvm.pagesize;
        meminfo[2] = (uint64_t)uvm.wired * uvm.pagesize;

        // cache is already included in active and inactive memory and
        // there's no way to know how much is in which -> disable cache
        meminfo[3] = 0;
        meminfo[4] = (uint64_t)uvm.free * uvm.pagesize;
    }
    if (pageinfo) {
        pageinfo[0] = (uint64_t)uvm.pgswapin;
        pageinfo[1] = (uint64_t)uvm.pgswapout;
    }
#else  /* HAVE_UVM */
    struct vmmeter vm;
#if defined(XOSVIEW_FREEBSD)
    safe_kvm_read_symbol(VMMETER_SYM_INDEX, &vm, sizeof(vm));
#else  /* XOSVIEW_DFBSD */
    struct vmstats vms;
    size_t size = sizeof(vms);
    if ( sysctlbyname("vm.vmstats", &vms, &size, NULL, 0) < 0 )
        logFatal << "sysctl vm.vmstats failed" << std::endl;
    size = sizeof(vm);
    if ( sysctlbyname("vm.vmmeter", &vm, &size, NULL, 0) < 0 )
        logFatal << "sysctl vm.vmmeter failed" << std::endl;
#endif
    if (meminfo) {
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
    }
    if (pageinfo) {
        pageinfo[0] = (uint64_t)vm.v_vnodepgsin + (uint64_t)vm.v_swappgsin;
        pageinfo[1] = (uint64_t)vm.v_vnodepgsout + (uint64_t)vm.v_swappgsout;
    }
#endif
}


// ------------------------  CPUMeter functions  -------------------------------

void BSDCPUInit() {
    OpenKDIfNeeded();
#if defined(XOSVIEW_FREEBSD)
    size_t size = sizeof(maxcpus);
    if ( sysctlbyname("kern.smp.maxcpus", &maxcpus, &size, NULL, 0) < 0 )
        logFatal << "sysctl kern.smp.maxcpus failed" << std::endl;
#elif defined(XOSVIEW_DFBSD)
    if ( kinfo_get_cpus(&maxcpus) )
        logFatal << "kinfo_get_cpus() failed" << std::endl;
#endif
}


void BSDGetCPUTimes(uint64_t *timeArray, unsigned int cpu) {
    // timeArray is CPUSTATES long.
    // cpu is the number of CPU to return, starting from 1. If cpu == 0,
    // return aggregate times for all CPUs.
    // All BSDs have separate calls for aggregate and separate times. Only
    // OpenBSD returns one CPU per call, others return all at once.
    if (!timeArray)
        logFatal << "BSDGetCPUTimes(): passed pointer was null." << std::endl;

    size_t size;
#if defined(XOSVIEW_DFBSD)
    size = sizeof(struct kinfo_cputime);
    std::vector<struct kinfo_cputime> times(maxcpus + 1, kinfo_cputime());
#elif defined(XOSVIEW_NETBSD)
    size = CPUSTATES * sizeof(uint64_t);
    std::vector<uint64_t> times((BSDCountCpus() + 1) * CPUSTATES, 0);
#elif defined(XOSVIEW_FREEBSD)
    size = CPUSTATES * sizeof(long);
    std::vector<long> times(maxcpus + 1, 0);
#else // XOSVIEW_OPENBSD
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
    timeArray[0] = times[cpu].cp_user;
    timeArray[1] = times[cpu].cp_nice;
    timeArray[2] = times[cpu].cp_sys;
    timeArray[3] = times[cpu].cp_intr;
    timeArray[4] = times[cpu].cp_idle;
#else  // !XOSVIEW_DFBSD
    size = CPUSTATES * sizeof(times[0]);
    if (cpu == 0) {  // aggregate times
#if defined(XOSVIEW_FREEBSD)
        if ( sysctlbyname("kern.cp_time", times, &size, NULL, 0) < 0 )
#else  // XOSVIEW_NETBSD || XOSVIEW_OPENBSD
        if ( sysctl(mib_cpt, 2, times.data(), &size, NULL, 0) < 0 )
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
        if ( sysctl(mib_cpt, 2, times.data() + CPUSTATES, &size, NULL, 0) < 0 )
            logFatal << "sysctl kern.cp_time failed" << std::endl;
#else  // XOSVIEW_OPENBSD
        mib_cpt2[2] = cpu - 1;
        if ( sysctl(mib_cpt2, 3, times.data(), &size, NULL, 0) < 0 )
            logFatal << "sysctl kern.cp_time2 failed" << std::endl;
#endif
    }
    for (int i = 0; i < CPUSTATES; i++)
#if defined(XOSVIEW_OPENBSD) // aggregates are long, singles uint64_t
        timeArray[i] = ( cpu ? times[i] : ((long*)(times))[i] );
#else  // XOSVIEW_FREEBSD || XOSVIEW_NETBSD
    timeArray[i] = times[cpu * CPUSTATES + i];
#endif
#endif
}


// ------------------------  NetMeter functions  -------------------------------
int BSDNetInit() {
    OpenKDIfNeeded();
#if defined(XOSVIEW_NETBSD)
    return ValidSymbol(IFNET_SYM_INDEX);
#else
    return 1;
#endif
}


void BSDGetNetInOut(uint64_t *inbytes, uint64_t *outbytes,
  const std::string &netIface, bool ignored) {

    std::string ifname;
    *inbytes = 0;
    *outbytes = 0;
#if defined(XOSVIEW_OPENBSD)
    size_t size;
    char *buf, *next;
    struct if_msghdr *ifm;
    struct if_data ifd;
    struct sockaddr_dl *sdl;

    if ( sysctl(mib_ifl, 6, NULL, &size, NULL, 0) < 0 )
        logFatal << "BSDGetNetInOut(): sysctl 1 failed" << std::endl;

    if ( (buf = (char *)malloc(size)) == NULL)
        logFatal << "BSDGetNetInOut(): malloc failed" << std::endl;

    if ( sysctl(mib_ifl, 6, buf, &size, NULL, 0) < 0 )
        logFatal << "BSDGetNetInOut(): sysctl 2 failed" << std::endl;

    for (next = buf; next < buf + size; next += ifm->ifm_msglen) {
        bool skipif = false;
        ifm = (struct if_msghdr *)next;
        if (ifm->ifm_type != RTM_IFINFO || (ifm->ifm_addrs & RTAX_IFP) == 0)
            continue;
        ifd = ifm->ifm_data;
        sdl = (struct sockaddr_dl *)(ifm + 1);
        if (sdl->sdl_family != AF_LINK)
            continue;
        if (netIface != "False") {
            ifname = std::string(sdl->sdl_data, 0, sdl->sdl_nlen);
            if ( (!ignored && ifname != netIface) ||
              ( ignored && ifname == netIface) )
                skipif = true;
        }
        if (!skipif) {
            *inbytes += ifd.ifi_ibytes;
            *outbytes += ifd.ifi_obytes;
        }
    }
    free(buf);
#else  /* XOSVIEW_OPENBSD */
    struct ifnet *ifnetp;
    struct ifnet ifnet;
#if defined (XOSVIEW_NETBSD)
    struct ifnet_head ifnethd;
#else
    struct ifnethead ifnethd;
#endif
    safe_kvm_read(nlst[IFNET_SYM_INDEX].n_value, &ifnethd, sizeof(ifnethd));
    ifnetp = TAILQ_FIRST(&ifnethd);

    while (ifnetp) {
        bool skipif = false;
        //  Now, dereference the pointer to get the ifnet struct.
        safe_kvm_read((unsigned long)ifnetp, &ifnet, sizeof(ifnet));
        ifname = std::string(ifnet.if_xname, 0, sizeof(ifname));
#if defined(XOSVIEW_NETBSD)
        ifnetp = TAILQ_NEXT(&ifnet, if_list);
#else
        ifnetp = TAILQ_NEXT(&ifnet, if_link);
#endif
        if (!(ifnet.if_flags & IFF_UP))
            continue;
        if (netIface != "False") {
            if ( (!ignored && netIface != ifname) ||
              ( ignored && netIface == ifname) )
                skipif = true;
        }
        if (!skipif) {
#if defined(XOSVIEW_DFBSD) && __DragonFly_version > 300304
            struct ifdata_pcpu *ifdatap = ifnet.if_data_pcpu;
            struct ifdata_pcpu ifdata;
            int ncpus = BSDCountCpus();
            for (int cpu = 0; cpu < ncpus; cpu++) {
                safe_kvm_read((unsigned long)ifdatap + cpu * sizeof(ifdata),
                  &ifdata, sizeof(ifdata));
                *inbytes  += ifdata.ifd_ibytes;
                *outbytes += ifdata.ifd_obytes;
            }
#else
            *inbytes  += ifnet.if_ibytes;
            *outbytes += ifnet.if_obytes;
#endif
        }
    }
#endif  /* XOSVIEW_OPENBSD */
}


//  ---------------------- Swap Meter stuff  -----------------------------------

int BSDSwapInit() {
    OpenKDIfNeeded();
    return 1;
}


void BSDGetSwapInfo(uint64_t *total, uint64_t *used) {
#if defined(HAVE_SWAPCTL)
    //  This code is based on a patch sent in by Scott Stevens
    //  (s.k.stevens@ic.ac.uk, at the time).
    struct swapent *sep, *swapiter;
    int bsize, rnswap, nswap = swapctl(SWAP_NSWAP, 0, 0);
    *total = *used = 0;

    if (nswap < 1)  // no swap devices on
        return;

    if ((sep = (struct swapent *)malloc(nswap* sizeof(struct swapent))) == NULL)
        logFatal << "BSDGetSwapInfo(): malloc failed" << std::endl;

    rnswap = swapctl(SWAP_STATS, (void *)sep, nswap);
    if (rnswap < 0)
        logFatal << "BSDGetSwapInfo(): getting SWAP_STATS failed" << std::endl;
    if (nswap != rnswap)
        logProblem << "SWAP_STATS gave different value than SWAP_NSWAP "
                   << "(nswap=" << nswap << " versus "
                   << "rnswap=" << rnswap << std::endl;

    swapiter = sep;
    bsize = 512; // block size is that of underlying device, *usually* 512 bytes
    for ( ; rnswap-- > 0; swapiter++) {
        *total += (uint64_t)swapiter->se_nblks * bsize;
        *used += (uint64_t)swapiter->se_inuse * bsize;
    }
    free(sep);
#else
    struct kvm_swap kswap;
    OpenKDIfNeeded();
    int pgsize = getpagesize();
    if ( kvm_getswapinfo(kd, &kswap, 1, 0) )
        logFatal << "BSDGetSwapInfo(): kvm_getswapinfo failed" << std::endl;

    *total = (uint64_t)kswap.ksw_total * pgsize;
    *used = (uint64_t)kswap.ksw_used * pgsize;
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


void DevStat_Init(void) {
    /*
     * Make sure that the userland devstat version matches the kernel
     * devstat version.
     */
#if defined(XOSVIEW_FREEBSD)
    if (devstat_checkversion(NULL) < 0) {
#else
    if (checkversion() < 0) {
#endif
        nodisk++;
        logProblem << devstat_errbuf << std::endl;
        return;
    }

    /* find out how many devices we have */
#if defined(XOSVIEW_FREEBSD)
    if ( (num_devices = devstat_getnumdevs(NULL)) < 0 ) {
#else
    if ( (num_devices = getnumdevs()) < 0 ) {
#endif
        nodisk++;
        logProblem << devstat_errbuf << std::endl;
        return;
    }

    cur.dinfo = (struct devinfo *)calloc(1, sizeof(struct devinfo));
    last.dinfo = (struct devinfo *)calloc(1, sizeof(struct devinfo));

    /*
     * Grab all the devices.  We don't look to see if the list has
     * changed here, since it almost certainly has.  We only look for
     * errors.
     */
#if defined(XOSVIEW_FREEBSD)
    if (devstat_getdevs(NULL, &cur) == -1) {
#else
    if (getdevs(&cur) == -1) {
#endif
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
#if defined(XOSVIEW_FREEBSD)
    if (devstat_buildmatch(da, &matches, &num_matches) != 0) {
#else
    if (buildmatch(da, &matches, &num_matches) != 0) {
#endif
        nodisk++;
        logProblem << devstat_errbuf;
        return;
    }

    if (num_matches == 0)
        select_mode = DS_SELECT_ADD;
    else
        select_mode = DS_SELECT_ONLY;

    /*
     * At this point, selectdevs will almost surely indicate that the
     * device list has changed, so we don't look for return values of 0
     * or 1.  If we get back -1, though, there is an error.
     */
#if defined(XOSVIEW_FREEBSD)
    if (devstat_selectdevs(&dev_select, &num_selected,
#else
    if (selectdevs(&dev_select, &num_selected,
#endif
        &num_selections, &select_generation,
        generation, cur.dinfo->devices, num_devices,
        matches, num_matches, NULL, 0, select_mode, 10, 0) == -1) {
        nodisk++;
        logProblem << devstat_errbuf << std::endl;
    }
}


uint64_t DevStat_Get(uint64_t *read_bytes, uint64_t *write_bytes) {
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
        *read_bytes += reads;
        *write_bytes += writes;
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


int BSDDiskInit() {
    OpenKDIfNeeded();
#if defined(HAVE_DEVSTAT)
    DevStat_Init();
#endif
    return 1;
}


uint64_t BSDGetDiskXFerBytes(uint64_t *read_bytes, uint64_t *write_bytes) {
#if defined(HAVE_DEVSTAT)
    return DevStat_Get(read_bytes, write_bytes);
#else
    *read_bytes = *write_bytes = 0;
# if defined(XOSVIEW_NETBSD)
    size_t size;
    // Do a sysctl with a NULL data pointer to get the size that would
    // have been returned, and use that to figure out # drives.
    if ( sysctl(mib_dsk, 3, NULL, &size, NULL, 0) < 0 )
        logFatal << "BSDGetDiskXFerBytes(): sysctl hw.iostats #1 failed\n";

    unsigned int ndrives = size / mib_dsk[2];
    struct io_sysctl drive_stats[ndrives];

    // Get the stats.
    if ( sysctl(mib_dsk, 3, drive_stats, &size, NULL, 0) < 0 )
        logFatal << "BSDGetDiskXFerBytes(): sysctl hw.iostats #2 failed\n";

    // Now accumulate the total.
    for (uint i = 0; i < ndrives; i++) {
        *read_bytes += drive_stats[i].rbytes;
        *write_bytes += drive_stats[i].wbytes;
    }
# else  /* XOSVIEW_OPENBSD */
    /*  This function is a little tricky -- we have to iterate over a
     *  list in kernel land.  To make things simpler, data structures
     *  and pointers for objects in kernel-land have kvm tacked on front
     *  of their names.  Thus, kvmdiskptr points to a disk struct in
     *  kernel memory.  kvmcurrdisk is a copy of the kernel's struct,
     *  and it has pointers in it to other structs, so it also is
     *  prefixed with kvm.
     */
    struct disklist_head kvmdisklist;
    struct disk *kvmdiskptr;
    struct disk kvmcurrdisk;
    safe_kvm_read_symbol(DISKLIST_SYM_INDEX, &kvmdisklist, sizeof(kvmdisklist));
    kvmdiskptr = TAILQ_FIRST(&kvmdisklist);
    while (kvmdiskptr != NULL) {
        safe_kvm_read((unsigned long)kvmdiskptr, &kvmcurrdisk,
          sizeof(kvmcurrdisk));
        *read_bytes += kvmcurrdisk.dk_rbytes;
        *write_bytes += kvmcurrdisk.dk_wbytes;
        kvmdiskptr = TAILQ_NEXT(&kvmcurrdisk, dk_link);
    }
# endif
#endif
    return (*read_bytes + *write_bytes);
}


//  ---------------------- Interrupt Meter stuff  ------------------------------

int BSDIntrInit() {
    OpenKDIfNeeded();
    // Make sure the intr counter array is nonzero in size.
#if defined(XOSVIEW_FREEBSD)
# if __FreeBSD_version >= 900040
    size_t nintr;
    safe_kvm_read(nlst[EINTRCNT_SYM_INDEX].n_value, &nintr, sizeof(nintr));
    return ValidSymbol(INTRCNT_SYM_INDEX)
        && ValidSymbol(EINTRCNT_SYM_INDEX) && (nintr > 0);
# else
    return ValidSymbol(INTRCNT_SYM_INDEX)
        && ValidSymbol(EINTRCNT_SYM_INDEX)
        && ((SymbolValue(EINTRCNT_SYM_INDEX)
            - SymbolValue(INTRCNT_SYM_INDEX)) > 0);
# endif
#elif defined(XOSVIEW_NETBSD)
    return ValidSymbol(ALLEVENTS_SYM_INDEX);
#endif
    return 1;
}


int BSDNumInts() {
    /* This code is stolen from vmstat. */
    int count = 0, nbr = 0;
#if defined(XOSVIEW_FREEBSD)
    size_t inamlen, nintr;

# if __FreeBSD_version >= 900040
    safe_kvm_read(nlst[EINTRCNT_SYM_INDEX].n_value, &nintr, sizeof(nintr));
    safe_kvm_read(nlst[EINTRNAMES_SYM_INDEX].n_value, &inamlen,
      sizeof(inamlen));
# else
    nintr = nlst[EINTRCNT_SYM_INDEX].n_value - nlst[INTRCNT_SYM_INDEX].n_value;
    inamlen = nlst[EINTRNAMES_SYM_INDEX].n_value
        - nlst[INTRNAMES_SYM_INDEX].n_value;
#  endif
    if (nintr == 0 || inamlen == 0) {
        logProblem << "Could not get interrupt numbers." << std::endl;
        return 0;
    }

    std::vector<char> intrvec(inamlen);
    safe_kvm_read(nlst[INTRNAMES_SYM_INDEX].n_value,
      intrvec.data(), intrvec.size());
    nintr /= sizeof(long);
    char *iname = intrvec.data();
    for (uint i = 0; i < nintr; i++) {
        if (iname) {
            std::istringstream is(iname);
            is >> util::sink("irq") >> nbr;
            if (is && nbr > count)
                count = nbr;
            iname += is.str().size() + 1;
        }
    }
#elif defined(XOSVIEW_NETBSD)
    struct evcntlist events;
    struct evcnt evcnt, *evptr;
    std::string dummy;
    //char *name;

    safe_kvm_read(nlst[ALLEVENTS_SYM_INDEX].n_value, &events,
      sizeof(events));
    evptr = TAILQ_FIRST(&events);
    while (evptr) {
        safe_kvm_read((unsigned long)evptr, &evcnt, sizeof(evcnt));
        if (evcnt.ev_type == EVCNT_TYPE_INTR) {
            std::vector<char> name(evcnt.ev_namelen + 1);
            safe_kvm_read((unsigned long)evcnt.ev_name,
              name.data(), name.size());
            std::istringstream is(name.data());
            is >> dummy >> nbr;
            if ( is && nbr > count )
                count = nbr;
        }
        evptr = TAILQ_NEXT(&evcnt, ev_list);
    }
#elif defined(XOSVIEW_OPENBSD)
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
#else  // XOSVIEW_DFBSD
    int nintr = 0;
    size_t inamlen;

    if ( sysctlbyname("hw.intrnames", NULL, &inamlen, NULL, 0) != 0 ) {
        logProblem << "sysctl hw.intrnames failed" << std::endl;
        return 0;
    }

    std::vector<char> inamev(inamlen);
    if ( sysctlbyname("hw.intrnames", inamev.data(), &inamlen,
        NULL, 0) < 0 ) {
        logProblem << "sysctl hw.intrnames failed" << std::endl;
        return 0;
    }
    for (size_t i = 0; i < inamev.size(); i++) {
        if (inamev[i] == '\0')  // count end-of-strings
            nintr++;
    }
    char *intrname = inamev.data();
    for (int i = 0; i < nintr; i++) {
        std::istringstream is(intrname);
        is >> util::sink("irq") >> nbr;
        if (is.fail()) {
            if ( ++nbr > count )  // unused ints are named irqn where
                count = nbr;      // 0<=n<=255, used ones have device name
        }
        intrname += is.str().size() + 1;
    }
#endif
    return count;  // this is the highest numbered interrupt
}


void BSDGetIntrStats(uint64_t *intrCount, unsigned int *intrNbrs) {
    /* This code is stolen from vmstat */
    int nbr = 0;
#if defined(XOSVIEW_FREEBSD)
    unsigned long *kvm_intrcnt, *intrcnt;
    char *kvm_intrnames, *intrnames;
    size_t inamlen, nintr;

# if __FreeBSD_version >= 900040
    safe_kvm_read(nlst[EINTRCNT_SYM_INDEX].n_value, &nintr, sizeof(nintr));
    safe_kvm_read(nlst[EINTRNAMES_SYM_INDEX].n_value,
      &inamlen, sizeof(inamlen));
# else
    nintr = nlst[EINTRCNT_SYM_INDEX].n_value - nlst[INTRCNT_SYM_INDEX].n_value;
    inamlen = nlst[EINTRNAMES_SYM_INDEX].n_value
        - nlst[INTRNAMES_SYM_INDEX].n_value;
# endif
    if (nintr == 0 || inamlen == 0) {
        logProblem << "Could not get interrupt numbers." << std::endl;
        return;
    }
    if ((kvm_intrcnt = (unsigned long *)malloc(nintr)) == NULL)
        logFatal << "BSDGetIntrStats(): malloc failed" << std::endl;
    if ((kvm_intrnames = (char *)malloc(inamlen)) == NULL)
        logFatal << "BSDGetIntrStats(): malloc failed" << std::endl;

    // keep track of the mem we're given:
    intrcnt = kvm_intrcnt;
    intrnames = kvm_intrnames;

    safe_kvm_read(nlst[INTRCNT_SYM_INDEX].n_value, kvm_intrcnt, nintr);
    safe_kvm_read(nlst[INTRNAMES_SYM_INDEX].n_value, kvm_intrnames, inamlen);

    nintr /= sizeof(long);
    /* kvm_intrname has the ASCII names of the IRQs, every null-terminated
     * string corresponds to a value in the kvm_intrcnt array
     * e.g. irq1: atkbd0
     */
    for (uint i = 0; i < nintr; i++) {
        /* Figure out which irq we have here */
        std::istringstream is(kvm_intrnames);
        is >> util::sink("irq") >> nbr;
        if (is) {
            intrCount[nbr] = *kvm_intrcnt;
            if (intrNbrs)
                intrNbrs[nbr] = 1;
        }
        kvm_intrcnt++;
        kvm_intrnames += is.str().size() + 1;
    }
    free(intrcnt);
    free(intrnames);
#elif defined(XOSVIEW_NETBSD)
    struct evcntlist events;
    struct evcnt evcnt, *evptr;
    char *name;

    safe_kvm_read(nlst[ALLEVENTS_SYM_INDEX].n_value, &events, sizeof(events));
    evptr = TAILQ_FIRST(&events);
    while (evptr) {
        safe_kvm_read((unsigned long)evptr, &evcnt, sizeof(evcnt));
        if (evcnt.ev_type == EVCNT_TYPE_INTR) {
            if ( !(name = (char *)malloc(evcnt.ev_namelen + 1)) )
                logFatal << "BSDGetIntrStats(): malloc failed" << std::endl;
            safe_kvm_read((unsigned long)evcnt.ev_name, name,
              evcnt.ev_namelen + 1);
            std::string dummy;
            std::istringstream is(name);
            is >> dummy >> nbr;
            if (is) {
                intrCount[nbr] = evcnt.ev_count;
                if (intrNbrs)
                    intrNbrs[nbr] = 1;
            }
            free(name);
        }
        evptr = TAILQ_NEXT(&evcnt, ev_list);
    }
#elif defined(XOSVIEW_OPENBSD)
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
        if (intrNbrs)
            intrNbrs[nbr] = 1;
    }
#else  // XOSVIEW_DFBSD
    int nintr = 0;
    size_t inamlen;
    char *intrs;

    if ( sysctlbyname("hw.intrnames", NULL, &inamlen, NULL, 0) != 0 ) {
        logProblem << "sysctl hw.intrnames failed" << std::endl;
        return;
    }

    std::vector<char> inamev(inamlen);
    if ( sysctlbyname("hw.intrnames", inamev.data(), &inamlen,
        NULL, 0) < 0 ) {
        logProblem << "sysctl hw.intrnames failed" << std::endl;
        return;
    }
    for (size_t i = 0; i < inamev.size(); i++) {
        if (inamev[i] == '\0')  // count end-of-strings
            nintr++;
    }

    std::vector<char *> inames(nintr, (char *)0);
    intrs = inamev.data();
    for (int i = 0; i < nintr; i++) {
        inames[i] = intrs;
        intrs += std::string(intrs).size() + 1;
    }
    std::vector<unsigned long> intrcnt(nintr, 0);
    inamlen = nintr * sizeof(long);
    if ( sysctlbyname("hw.intrcnt", intrcnt.data(), &inamlen, NULL, 0) < 0 )
        logFatal << "sysctl hw.intrcnt failed" << std::endl;

    for (int i = 0; i < nintr; i++) {
        std::istringstream is(inames[i]);
        is >> util::sink("irq") >> nbr;
        if (!is) {
            nbr++;
            intrCount[nbr] += intrcnt[i];
            if (intrNbrs)
                intrNbrs[nbr] = 1;
        }
    }
#endif
}


//  ---------------------- Sensor Meter stuff  ---------------------------------

static int mib_cpu[2] = { CTL_HW, HW_NCPU };

int BSDCountCpus(void) {
    int cpus = 0;
    size_t size = sizeof(cpus);
    if ( sysctl(mib_cpu, 2, &cpus, &size, NULL, 0) < 0 )
        logProblem << "sysctl hw.ncpu failed." << std::endl;
    return cpus;
}


#if defined(__i386__) || defined(__x86_64__)
unsigned int BSDGetCPUTemperature(float *temps, float *tjmax) {
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
                if (temps)
                    temps[i] = (prop_number_integer_value(
                          (prop_number_t)pobj1) / 1000000.0) - 273.15;
                nbr++;
            }
            if ( (pobj2 = prop_dictionary_get((prop_dictionary_t)pobj,
                  "critical-max")) && tjmax )
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
            if (temps)
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
            if (temps)
#if __FreeBSD_version >= 702106
                temps[i] = ((float)val - 2732.0) / 10.0;
#else
                temps[i] = (float)val;
#endif
        }
        else
            logProblem << "sysctl " << name << " failed" << std::endl;

        if (tjmax) {
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
    }
#endif
#endif
    return nbr;
}
#endif


void BSDGetSensor(const std::string &name, const std::string &valname,
  float *value, std::string &unit) {

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
            *value = (val / 1000000.0) - 273.15;  // temps are in microkelvins
            unit = "\260C";
        }
        else if ( ptype == "Fan") {
            *value = (float)val;                  // plain integer value
            unit = "RPM";
        }
        else if ( ptype == "Integer" )
            *value = (float)val;                  // plain integer value
        else if ( ptype == "Voltage" ) {
            *value = (float)val / 1000000.0;      // units are micro{V,A,W,Ohm}
            unit = "V";
        }
        else if ( ptype == "Ampere hour" ) {
            *value = (float)val / 1000000.0;      // units are micro{V,A,W,Ohm}
            unit = "Ah";
        }
        else if ( ptype == "Ampere" ) {
            *value = (float)val / 1000000.0;      // units are micro{V,A,W,Ohm}
            unit = "A";
        }
        else if ( ptype == "Watt hour" ) {
            *value = (float)val / 1000000.0;      // units are micro{V,A,W,Ohm}
            unit = "Wh";
        }
        else if ( ptype == "Watts" ) {
            *value = (float)val / 1000000.0;      // units are micro{V,A,W,Ohm}
            unit = "W";
        }
        else if ( ptype == "Ohms" ) {
            *value = (float)val / 1000000.0;      // units are micro{V,A,W,Ohm}
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
        *value = ((float)val - 2732.0) / 10.0;
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
                    *value = (float)(s.value - 273150000) / 1000000.0;
                    unit = "\260C";
                    break;
                case SENSOR_FANRPM:
                    *value = (float)s.value;
                    unit = "RPM";
                    break;
                case SENSOR_VOLTS_DC:
                case SENSOR_VOLTS_AC:
                    *value = (float)s.value / 1000000.0;
                    unit = "V";
                    break;
                case SENSOR_OHMS:
                    *value = (float)s.value;
                    unit = "Ohm";
                    break;
                case SENSOR_WATTS:
                    *value = (float)s.value / 1000000.0;
                    unit = "W";
                    break;
                case SENSOR_AMPS:
                    *value = (float)s.value / 1000000.0;
                    unit = "A";
                    break;
                case SENSOR_WATTHOUR:
                    *value = (float)s.value / 1000000.0;
                    unit = "Wh";
                    break;
                case SENSOR_AMPHOUR:
                    *value = (float)s.value / 1000000.0;
                    unit = "Ah";
                    break;
                case SENSOR_PERCENT:
                    *value = (float)s.value / 1000.0;
                    unit = "%";
                    break;
                case SENSOR_LUX:
                    *value = (float)s.value / 1000000.0;
                    unit = "lx";
                    break;
                case SENSOR_TIMEDELTA:
                    *value = (float)s.value / 1000000000.0;
                    unit = "s";
                    break;
#if defined(XOSVIEW_OPENBSD)
                case SENSOR_HUMIDITY:
                    *value = (float)s.value / 1000.0;
                    unit = "%";
                    break;
                case SENSOR_FREQ:
                    *value = (float)s.value / 1000000.0;
                    unit = "Hz";
                    break;
                case SENSOR_ANGLE:
                    *value = (float)s.value / 1000000.0;
                    unit = "\260";
                    break;
#if OpenBSD > 201211
                case SENSOR_DISTANCE:
                    *value = (float)s.value / 1000000.0;
                    unit = "m";
                    break;
                case SENSOR_PRESSURE:
                    *value = (float)s.value / 1000.0;
                    unit = "Pa";
                    break;
                case SENSOR_ACCEL:
                    *value = (float)s.value / 1000000.0;
                    unit = "m\\/s\262"; // m/s²
                    break;
#endif
#endif
                case SENSOR_INDICATOR:
                case SENSOR_INTEGER:
                case SENSOR_DRIVE:
                default:
                    *value = (float)s.value;
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
    BSDGetSensor("acpibat0", "amphour0", &val, emptyStr);
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


void BSDGetBatteryInfo(int *remaining, unsigned int *state) {
    *state = XOSVIEW_BATT_NONE;
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
                        *state |= XOSVIEW_BATT_CHARGING;
            }
            else if (std::string(name, 0, 14) == "discharge rate") {
                // discharging or not?
                if ( (pobj1 = prop_dictionary_get((prop_dictionary_t)pobj,
                      "cur-value")) )
                    if ( prop_number_integer_value((prop_number_t)pobj1) )
                        *state |= XOSVIEW_BATT_DISCHARGING;
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
        BSDGetSensor(battery, "amphour0", &val, emptyStr); // full capacity
        if (val < 0) // no more batteries
            break;
        batteries++;
        total_capacity += val;
        emptyStr = "";
        BSDGetSensor(battery, "amphour1", &val, emptyStr); // warning capacity
        total_low += val;
        emptyStr = "";
        BSDGetSensor(battery, "amphour2", &val, emptyStr); // low capacity
        total_crit += val;
        emptyStr = "";
        BSDGetSensor(battery, "amphour3", &val, emptyStr); // remaining
        total_charge += val;
        emptyStr = "";
        BSDGetSensor(battery, "raw0", &val, emptyStr); // state
        if ((int)val == 1)
            *state |= XOSVIEW_BATT_DISCHARGING;
        else if ((int)val == 2)
            *state |= XOSVIEW_BATT_CHARGING;
        // there's also 0 state for idle/full
    }
#endif
    if (batteries == 0) { // all batteries are off
        *state = XOSVIEW_BATT_NONE;
        *remaining = 0;
        return;
    }
    *remaining = 100 * total_charge / total_capacity;
    if ( !(*state & XOSVIEW_BATT_CHARGING) &&
      !(*state & XOSVIEW_BATT_DISCHARGING) )
        *state |= XOSVIEW_BATT_FULL;  // full when not charging nor discharging
    if (total_capacity < total_low)
        *state |= XOSVIEW_BATT_LOW;
    if (total_capacity < total_crit)
        *state |= XOSVIEW_BATT_CRITICAL;
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
            *remaining = aip.ai_batt_life; // only 0-100 are valid values
        else
            *remaining = 0;
        if (aip.ai_batt_stat == 0)
            *state |= XOSVIEW_BATT_FULL;
        else if (aip.ai_batt_stat == 1)
            *state |= XOSVIEW_BATT_LOW;
        else if (aip.ai_batt_stat == 2)
            *state |= XOSVIEW_BATT_CRITICAL;
        else if (aip.ai_batt_stat == 3)
            *state |= XOSVIEW_BATT_CHARGING;
        else
            *state = XOSVIEW_BATT_NONE;
        return;
    }
    // ACPI
    union acpi_battery_ioctl_arg battio;
    battio.unit = ACPI_BATTERY_ALL_UNITS;
    if ( ioctl(fd, ACPIIO_BATT_GET_BATTINFO, &battio) == -1 )
        logFatal << "failed to get ACPI battery info" << std::endl;
    if ( close(fd) == -1 )
        logFatal << "Could not close " << ACPIDEV << std::endl;
    *remaining = battio.battinfo.cap;
    if (battio.battinfo.state != ACPI_BATT_STAT_NOT_PRESENT) {
        if (battio.battinfo.state == 0)
            *state |= XOSVIEW_BATT_FULL;
        if (battio.battinfo.state & ACPI_BATT_STAT_CRITICAL)
            *state |= XOSVIEW_BATT_CRITICAL;
        if (battio.battinfo.state & ACPI_BATT_STAT_DISCHARG)
            *state |= XOSVIEW_BATT_DISCHARGING;
        if (battio.battinfo.state & ACPI_BATT_STAT_CHARGING)
            *state |= XOSVIEW_BATT_CHARGING;
    }
#endif
}
