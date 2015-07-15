//
// $Id: cpumeter.cc,v 1.7 2006/02/18 07:57:21 romberg Exp $
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
#include "cpumeter.h"

#include <sstream>


CPUMeter::CPUMeter(XOSView *parent, kstat_ctl_t *_kc, int cpuid)
    : FieldMeterGraph(parent, CPU_STATES, util::toupper(cpuStr(cpuid)),
      "USER/SYS/WAIT/IDLE") {

    kc = _kc;
    for (int i = 0 ; i < 2 ; i++)
        for (int j = 0 ; j < CPU_STATES ; j++)
            cputime_[i][j] = 0;
    cpuindex_ = 0;

    int j = 0;
    for (ksp = kc->kc_chain; ksp != NULL; ksp = ksp->ks_next) {
        if (std::string(ksp->ks_name).substr(0, 8) == "cpu_stat") {
            j++;
            if (j == cpuid)
                break;
        }
    }
}

CPUMeter::~CPUMeter(void) {
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
}

void CPUMeter::checkevent(void) {
    getcputime();
    drawfields(parent_->g());
}

void CPUMeter::getcputime(void) {
    total_ = 0;
    cpu_stat_t cs;

    if (kstat_read(kc, ksp, &cs) == -1)
        logFatal << "kstat_read() failed" << std::endl;

    cputime_[cpuindex_][0] = cs.cpu_sysinfo.cpu[CPU_USER];
    cputime_[cpuindex_][1] = cs.cpu_sysinfo.cpu[CPU_KERNEL];
    cputime_[cpuindex_][2] = cs.cpu_sysinfo.cpu[CPU_WAIT];
    cputime_[cpuindex_][3] = cs.cpu_sysinfo.cpu[CPU_IDLE];

    int oldindex = (cpuindex_ + 1) % 2;
    for (int i = 0 ; i < CPU_STATES ; i++) {
        fields_[i] = cputime_[cpuindex_][i] - cputime_[oldindex][i];
        total_ += fields_[i];
    }
    cpuindex_ = (cpuindex_ + 1) % 2;

    if (total_)
        setUsed(total_ - fields_[3], total_);
}

std::string CPUMeter::cpuStr(int num) {
    std::ostringstream str;

    str << "cpu";
    if (num != 0)
        str << (num - 1);

    return str.str();
}

int CPUMeter::countCPUs(kstat_ctl_t *kc) {
    kstat_t *ksp;
    int i = 0;

    for (ksp = kc->kc_chain; ksp != NULL; ksp = ksp->ks_next) {
        if (std::string(ksp->ks_name).substr(0, 8) == "cpu_stat")
            i++;
    }
    return (i);
}
