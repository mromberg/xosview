//
//  Copyright (c) 1994, 1995, 2015, 2016
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

#if defined(XOSVIEW_DFBSD)
#include <kinfo.h>
#endif

#include <sys/resource.h>


CPUMeter::CPUMeter( unsigned int nbr )
    : FieldMeterGraph( 5, "CPU", "USR/NICE/SYS/INT/FREE" ),
      _cputime(2, std::vector<uint64_t>(5, 0)),
      _cpuindex(0), _nbr(nbr) {

    if (_nbr)
        title(std::string("CPU") + util::repr(_nbr - 1));
}


CPUMeter::~CPUMeter( void ) {
}


void CPUMeter::checkResources(const ResDB &rdb) {
    FieldMeterGraph::checkResources(rdb);

    setfieldcolor( 0, rdb.getColor("cpuUserColor") );
    setfieldcolor( 1, rdb.getColor("cpuNiceColor") );
    setfieldcolor( 2, rdb.getColor("cpuSystemColor") );
    setfieldcolor( 3, rdb.getColor("cpuInterruptColor") );
    setfieldcolor( 4, rdb.getColor("cpuFreeColor") );
}


void CPUMeter::checkevent( void ) {
    getcputime();
}


void CPUMeter::getcputime( void ) {
    std::vector<uint64_t> tempCPU;
    total_ = 0;

    getCPUTimes(tempCPU, _nbr);

    int oldindex = (_cpuindex + 1) % 2;
    for (size_t i = 0 ; i < _cputime[_cpuindex].size() ; i++) {
        _cputime[_cpuindex][i] = tempCPU[i];
        fields_[i] = _cputime[_cpuindex][i] - _cputime[oldindex][i];
        total_ += fields_[i];
    }
    if (total_) {
        setUsed(total_ - fields_[4], total_);
        _cpuindex = (_cpuindex + 1) % 2;
    }
}


size_t CPUMeter::countCPUs(void) {

    const int mib_cpu[2] = { CTL_HW, HW_NCPU };
    static SysCtl ncpu_sc(mib_cpu, 2);
    static int cpus = -1;

    if (cpus == -1) {
        if (!ncpu_sc.get(cpus))
            logFatal << "sysctl(" << ncpu_sc.id() << "failed." << std::endl;
    }

    return cpus;
}


#if defined(XOSVIEW_DFBSD)
void CPUMeter::getCPUTimes(std::vector<uint64_t> &timeArray,
  size_t cpu) {

    static SysCtl cputime_sc("kern.cputime");  // per-cpu.
    static SysCtl cp_time_sc("kern.cp_time");  // aggregate.

    timeArray.resize(CPUSTATES);

    if (cpu) {
        std::vector<struct kinfo_cputime> times(countCPUs(),
          kinfo_cputime());
        if (!cputime_sc.get(times))
            logFatal << "sysctl(" << cputime_sc.id() << ") failed."
                     << std::endl;

        cpu -= 1;  // cpu starts at 1.  Stats start at 0.
        timeArray[0] = times[cpu].cp_user;
        timeArray[1] = times[cpu].cp_nice;
        timeArray[2] = times[cpu].cp_sys;
        timeArray[3] = times[cpu].cp_intr;
        timeArray[4] = times[cpu].cp_idle;
    }
    else { // aggregate.
        std::vector<long> times(CPUSTATES);
        if (!cp_time_sc.get(times))
            logFatal << "sysctl(" << cp_time_sc.id() << ") failed."
                     << std::endl;

        std::copy(times.begin(), times.end(), timeArray.begin());
    }
}
#endif


#if defined(XOSVIEW_FREEBSD)
void CPUMeter::getCPUTimes(std::vector<uint64_t> &timeArray, size_t cpu) {

    static SysCtl cp_time_sc("kern.cp_time");   // aggregate times.
    static SysCtl cp_times_sc("kern.cp_times"); // per-cpu times.

    timeArray.resize(CPUSTATES);

    if (cpu) {
        std::vector<long> times(countCPUs() * CPUSTATES, 0);
        if (!cp_times_sc.get(times))
            logFatal << "sysctl(" << cp_times_sc.id() << ") failed."
                     << std::endl;

        cpu -= 1;  // cpu starts at 1 and index starts at 0.

        for (size_t i = 0 ; i < timeArray.size() ; i++)
            timeArray[i] = times[cpu * CPUSTATES + i];
    }
    else {  // aggregate
        std::vector<long> times(CPUSTATES, 0);
        if (!cp_time_sc.get(times))
            logFatal << "sysctl(" << cp_time_sc.id() << ") failed."
                     << std::endl;

        std::copy(times.begin(), times.end(), timeArray.begin());
    }
}
#endif
