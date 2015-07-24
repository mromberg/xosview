//
//  Copyright (c) 2015
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//

#include "cpumeter.h"

#include <sys/sysinfo.h>



CPUMeter::CPUMeter(XOSView *parent, kstat_ctl_t *kc, int cpuid)
    : FieldMeterGraph(parent, CPU_STATES, cpuStr(cpuid), "USER/SYS/WAIT/IDLE"),
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

void CPUMeter::checkResources(void) {

    FieldMeterGraph::checkResources();

    setfieldcolor(0, parent_->getResource("cpuUserColor"));
    setfieldcolor(1, parent_->getResource("cpuSystemColor"));
    setfieldcolor(2, parent_->getResource("cpuInterruptColor"));
    setfieldcolor(3, parent_->getResource("cpuFreeColor"));
    priority_ = util::stoi(parent_->getResource("cpuPriority"));
    dodecay_ = parent_->isResourceTrue("cpuDecay");
    useGraph_ = parent_->isResourceTrue("cpuGraph");
    setUsedFormat(parent_->getResource("cpuUsedFormat"));
    decayUsed(parent_->isResourceTrue("cpuUsedDecay"));
}


void CPUMeter::checkevent(void) {
    getcputime();
    drawfields(parent_->g());
}


void CPUMeter::getcputime(void) {
    total_ = 0;
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
        fields_[i] = cputime_[cpuindex_][i] - cputime_[oldindex][i];
        total_ += fields_[i];
    }
    cpuindex_ = (cpuindex_ + 1) % 2;

    if (total_)
        setUsed(total_ - fields_[3], total_);
}


const char *CPUMeter::cpuStr(int num) {
    static char buffer[8] = "CPU";
    if (num >= 0)
        snprintf(buffer + 3, 4, "%d", num);
    buffer[7] = '\0';
    return buffer;
}
