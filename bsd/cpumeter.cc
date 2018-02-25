//
//  Copyright (c) 1994, 1995, 2015, 2016, 2018
//  by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  NetBSD port:
//  Copyright (c) 1995, 1996, 1997-2002 by Brian Grayson (bgrayson@netbsd.org)
//
//  This file was written by Brian Grayson for the NetBSD and xosview
//    projects.
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//
#include "cpumeter.h"
#include "sctl.h"
#include "scache.h"

#if defined(XOSVIEW_DFBSD)
#include <kinfo.h>
#endif

#include <sys/resource.h>
#if defined(HAVE_SYS_SCHED_H)
#include <sys/sched.h>
#endif


CPUMeter::CPUMeter(size_t nbr)
    : FieldMeterGraph(5, "CPU", "USR/NICE/SYS/INT/FREE"),
      _cputime(2, std::vector<uint64_t>(5, 0)),
      _cpuindex(0), _nbr(nbr) {

    if (_nbr)
        title("CPU" + std::to_string(_nbr - 1));
}


void CPUMeter::checkResources(const ResDB &rdb) {
    FieldMeterGraph::checkResources(rdb);

    setfieldcolor(0, rdb.getColor("cpuUserColor"));
    setfieldcolor(1, rdb.getColor("cpuNiceColor"));
    setfieldcolor(2, rdb.getColor("cpuSystemColor"));
    setfieldcolor(3, rdb.getColor("cpuInterruptColor"));
    setfieldcolor(4, rdb.getColor("cpuFreeColor"));
}


void CPUMeter::checkevent(void) {
    const auto tempCPU = getStats();
    _total = 0;

    auto &oldcput = _cputime[(_cpuindex + 1) % 2];
    auto &cput = _cputime[_cpuindex];
    for (size_t i = 0 ; i < cput.size() ; i++) {
        cput[i] = tempCPU[i];
        _fields[i] = cput[i] - oldcput[i];
        _total += _fields[i];
    }
    if (_total) {
        setUsed(_total - _fields[4], _total);
        _cpuindex = (_cpuindex + 1) % 2;
    }
}


size_t CPUMeter::countCPUs(void) {

    static SysCtl ncpu_sc = { CTL_HW, HW_NCPU };
    static int cpus = -1;

    if (cpus == -1) {
        if (!ncpu_sc.get(cpus))
            logFatal << "sysctl(" << ncpu_sc.id() << "failed." << std::endl;
    }

    return cpus;
}


const std::vector<uint64_t> &CPUMeter::getStats(void) const {

    static StatCache<std::vector<std::vector<uint64_t>>> sc;

    if (!sc.valid())
        sc.set(readStats());

    return sc.get()[_nbr];
}


#if defined(XOSVIEW_DFBSD)
std::vector<std::vector<uint64_t> > CPUMeter::readStats(void) const {

    static SysCtl cputime_sc("kern.cputime");  // per-cpu.
    static SysCtl cp_time_sc("kern.cp_time");  // aggregate.

    const size_t ncpus = countCPUs();
    // index0 = aggregate, index1 = first cpu, ...
    std::vector<std::vector<uint64_t>> rval(ncpus + 1,
      std::vector<uint64_t>(CPUSTATES));

    std::vector<struct kinfo_cputime> times(countCPUs(),
      kinfo_cputime());
    if (!cputime_sc.get(times))
        logFatal << "sysctl(" << cputime_sc.id() << ") failed."
                 << std::endl;

    for (size_t cpu = 0 ; cpu < ncpus ; cpu++) {
        rval[cpu + 1][0] = times[cpu].cp_user;
        rval[cpu + 1][1] = times[cpu].cp_nice;
        rval[cpu + 1][2] = times[cpu].cp_sys;
        rval[cpu + 1][3] = times[cpu].cp_intr;
        rval[cpu + 1][4] = times[cpu].cp_idle;
    }

    // aggregate.
    std::vector<long> atimes(CPUSTATES);
    if (!cp_time_sc.get(atimes))
        logFatal << "sysctl(" << cp_time_sc.id() << ") failed."
                 << std::endl;

    std::copy(atimes.begin(), atimes.end(), rval[0].begin());

    return rval;
}
#endif


#if defined(XOSVIEW_FREEBSD)
std::vector<std::vector<uint64_t> > CPUMeter::readStats(void) const {

    static SysCtl cp_time_sc("kern.cp_time");   // aggregate times.
    static SysCtl cp_times_sc("kern.cp_times"); // per-cpu times.

    const size_t ncpus = countCPUs();
    // index0 = aggregate, index1 = first cpu, ...
    std::vector<std::vector<uint64_t>> rval(ncpus + 1,
      std::vector<uint64_t>(CPUSTATES));


    std::vector<long> times(countCPUs() * CPUSTATES, 0);
    if (!cp_times_sc.get(times))
        logFatal << "sysctl(" << cp_times_sc.id() << ") failed."
                 << std::endl;

    for (size_t cpu = 0 ; cpu < ncpus ; cpu++)
        std::copy(times.begin() + CPUSTATES * cpu,
          times.begin() + CPUSTATES * (cpu + 1),
          rval[cpu + 1].begin());

    // aggregate
    std::vector<long> atimes(CPUSTATES, 0);
    if (!cp_time_sc.get(atimes))
        logFatal << "sysctl(" << cp_time_sc.id() << ") failed."
                 << std::endl;

    std::copy(atimes.begin(), atimes.end(), rval[0].begin());

    return rval;
}
#endif


#if defined(XOSVIEW_NETBSD)
std::vector<std::vector<uint64_t>> CPUMeter::readStats(void) const {

    //------- NetBSD docs ------
    // Returns an array of CPUSTATES uint64_ts.  This array contains the
    // number of clock ticks spent in different CPU states.  On multi-
    // processor systems, the sum across all CPUs is returned unless
    // appropriate space is given for one data set for each CPU.  Data
    // for a specific CPU can also be obtained by adding the number of
    // the CPU at the end of the MIB, enlarging it by one.
    //------- NetBSD docs ------

    static SysCtl cp_time_sc("kern.cp_time");

    const size_t ncpus = countCPUs();
    // index0 = aggregate, index1 = first cpu, ...
    std::vector<std::vector<uint64_t>> rval(ncpus + 1,
      std::vector<uint64_t>(CPUSTATES));

    cp_time_sc.mib().resize(3);
    for (size_t cpu = 0 ; cpu < ncpus ; cpu++) {
        cp_time_sc.mib()[2] = cpu;
        if (!cp_time_sc.get(rval[cpu + 1]))
            logFatal << "sysctl(" << cp_time_sc.id() << ") failed."
                     << std::endl;
    }

    // aggregate.
    cp_time_sc.mib().resize(2);
    if (!cp_time_sc.get(rval[0]))
        logFatal << "sysctl(" << cp_time_sc.id() << ") failed."
                 << std::endl;

    return rval;
}
#endif


#if defined(XOSVIEW_OPENBSD)
std::vector<std::vector<uint64_t> > CPUMeter::readStats(void) const {

    //---- OpenBSD docs ----
    // KERN_CPTIME  long[CPUSTATES]
    //    An array of longs of size CPUSTATES is returned, containing
    //    statistics about the number of ticks spent by the system in
    //    interrupt processing, user processes (nice(1) or normal), system
    //    processing, or idling.
    // KERN_CPTIME2  u_int64_t[CPUSTATES]
    //    Similar to KERN_CPTIME, but obtains information from only the
    //    single CPU specified by the third level name given.
    //---- OpenBSD docs ----

    const std::array<int, 2> mib_cpt = {{ CTL_KERN, KERN_CPTIME }};
    const std::array<int, 3> mib_cpt2 = {{ CTL_KERN, KERN_CPTIME2, 0 }};
    static SysCtl cp_time_sc(mib_cpt.data(), mib_cpt.size());
    static SysCtl cp_time2_sc(mib_cpt2.data(), mib_cpt2.size());

    const size_t ncpus = countCPUs();
    // index0 = aggregate, index1 = first cpu, ...
    std::vector<std::vector<uint64_t>> rval(ncpus + 1,
      std::vector<uint64_t>(CPUSTATES));

    for (size_t cpu = 0 ; cpu < ncpus ; cpu++) {
        cp_time2_sc.mib()[2] = cpu;

        if (!cp_time2_sc.get(rval[cpu + 1]))
            logFatal << "sysctl(" << cp_time2_sc.id() << ") failed."
                     << std::endl;
    }

    // aggregate.
    std::vector<long> times(CPUSTATES, 0);
    if (!cp_time_sc.get(times))
        logFatal << "sysctl(" << cp_time_sc.id() << ") failed."
                 << std::endl;

    std::copy(times.begin(), times.end(), rval[0].begin());

    return rval;
}
#endif
