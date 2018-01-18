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
      cputime_(2, std::vector<float>(CPU_STATES, 0.0)), cpuindex_(0),
      _cpustats(KStatList::getList(kc, KStatList::CPU_STAT)),
      _aggregate(cpuid < 0), _kc(kc), _ksp(0) {

    if (!_aggregate)
        for (unsigned int i = 0; i < _cpustats->count(); i++)
            if ((*_cpustats)[i]->ks_instance == cpuid)
                _ksp = (*_cpustats)[i];
}


CPUMeter::~CPUMeter(void){
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
        cputime_[cpuindex_].assign(CPU_STATES, 0.0);
        for (unsigned int i = 0; i < _cpustats->count(); i++) {
            if (kstat_read(_kc, (*_cpustats)[i], &cs) == -1)
                logFatal << "kstat_read() failed." << std::endl;

            cputime_[cpuindex_][0] += cs.cpu_sysinfo.cpu[CPU_USER];
            cputime_[cpuindex_][1] += cs.cpu_sysinfo.cpu[CPU_KERNEL];
            cputime_[cpuindex_][2] += cs.cpu_sysinfo.cpu[CPU_WAIT];
            cputime_[cpuindex_][3] += cs.cpu_sysinfo.cpu[CPU_IDLE];
        }
    }
    else {
        if (kstat_read(_kc, _ksp, &cs) == -1)
            logFatal << "kstat_read() failed." << std::endl;

        cputime_[cpuindex_][0] = cs.cpu_sysinfo.cpu[CPU_USER];
        cputime_[cpuindex_][1] = cs.cpu_sysinfo.cpu[CPU_KERNEL];
        cputime_[cpuindex_][2] = cs.cpu_sysinfo.cpu[CPU_WAIT];
        cputime_[cpuindex_][3] = cs.cpu_sysinfo.cpu[CPU_IDLE];
    }

    int oldindex = (cpuindex_ + 1) % 2;
    for (int i = 0 ; i < CPU_STATES ; i++) {
        _fields[i] = cputime_[cpuindex_][i] - cputime_[oldindex][i];
        _total += _fields[i];
    }
    cpuindex_ = (cpuindex_ + 1) % 2;

    if (_total)
        setUsed(_total - _fields[3], _total);
}


std::string CPUMeter::cpuStr(int num) {
    if (num < 0)
        return "CPU";

    return std::string("CPU" + util::repr(num));
}
