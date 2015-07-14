//
//  Copyright (c) 2015
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//

#ifndef CPUMETER_H
#define CPUMETER_H

#include "fieldmetergraph.h"

#include <kstat.h>
#include <sys/sysinfo.h>

class CPUMeter : public FieldMeterGraph {
public:
    CPUMeter(XOSView *parent, kstat_ctl_t *kcp, int cpuid = 0);
    ~CPUMeter(void);

    virtual std::string name(void) const { return "CPUMeter"; }
    void checkevent(void);

    void checkResources(void);

    static int countCPUs(kstat_ctl_t *kc);
    static const char *cpuStr(int num);

protected:
    float cputime_[2][CPU_STATES];
    int cpuindex_;

    void getcputime(void);
    const char *toUpper(const char *str);

private:
    kstat_ctl_t *kc;
    kstat_t *ksp;
};

#endif
