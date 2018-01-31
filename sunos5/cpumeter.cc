//
//  Copyright (c) 2015, 2018
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//

#include "cpumeter.h"

#include <sys/sysinfo.h>



CPUMeter::CPUMeter(kstat_ctl_t *kc, int cpuid)
    : FieldMeterGraph(CPU_STATES, cpuStr(cpuid), "USER/SYS/WAIT/IDLE"),
      _cputime(2, std::vector<float>(CPU_STATES, 0.0)), _cpuindex(0),
      _cpustats(KStatList::getList(kc, KStatList::CPU_STAT)),
      _aggregate(cpuid < 0), _kc(kc), _ksp(nullptr) {

    if (!_aggregate)
        for (size_t i = 0; i < _cpustats->count(); i++)
            if ((*_cpustats)[i]->ks_instance == cpuid)
                _ksp = (*_cpustats)[i];
}


void CPUMeter::checkResources(const ResDB &rdb) {

    FieldMeterGraph::checkResources(rdb);

    setfieldcolor(0, rdb.getColor("cpuUserColor"));
    setfieldcolor(1, rdb.getColor("cpuSystemColor"));
    setfieldcolor(2, rdb.getColor("cpuInterruptColor"));
    setfieldcolor(3, rdb.getColor("cpuFreeColor"));
}


void CPUMeter::checkevent(void) {
    getcputime();
}


void CPUMeter::getcputime(void) {
    _total = 0;
    cpu_stat_t cs;

    if (_aggregate) {
        _cpustats->update(_kc);
        _cputime[_cpuindex].assign(CPU_STATES, 0.0);
        for (unsigned int i = 0; i < _cpustats->count(); i++) {
            if (kstat_read(_kc, (*_cpustats)[i], &cs) == -1)
                logFatal << "kstat_read() failed." << std::endl;

            _cputime[_cpuindex][0] += cs.cpu_sysinfo.cpu[CPU_USER];
            _cputime[_cpuindex][1] += cs.cpu_sysinfo.cpu[CPU_KERNEL];
            _cputime[_cpuindex][2] += cs.cpu_sysinfo.cpu[CPU_WAIT];
            _cputime[_cpuindex][3] += cs.cpu_sysinfo.cpu[CPU_IDLE];
        }
    }
    else {
        if (kstat_read(_kc, _ksp, &cs) == -1)
            logFatal << "kstat_read() failed." << std::endl;

        _cputime[_cpuindex][0] = cs.cpu_sysinfo.cpu[CPU_USER];
        _cputime[_cpuindex][1] = cs.cpu_sysinfo.cpu[CPU_KERNEL];
        _cputime[_cpuindex][2] = cs.cpu_sysinfo.cpu[CPU_WAIT];
        _cputime[_cpuindex][3] = cs.cpu_sysinfo.cpu[CPU_IDLE];
    }

    const size_t oldindex = (_cpuindex + 1) % 2;
    for (int i = 0 ; i < CPU_STATES ; i++) {
        _fields[i] = _cputime[_cpuindex][i] - _cputime[oldindex][i];
        _total += _fields[i];
    }
    _cpuindex = (_cpuindex + 1) % 2;

    if (_total)
        setUsed(_total - _fields[3], _total);
}


std::string CPUMeter::cpuStr(int num) {
    if (num < 0)
        return "CPU";

    return std::string("CPU" + util::repr(num));
}
