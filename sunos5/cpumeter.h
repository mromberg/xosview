//
//  Copyright (c) 2015
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//

#ifndef CPUMETER_H
#define CPUMETER_H

#include "fieldmetergraph.h"
#include "kstats.h"



class CPUMeter : public FieldMeterGraph {
public:
    CPUMeter(XOSView *parent, kstat_ctl_t *kcp, int cpuid = 0);
    ~CPUMeter(void);

    virtual std::string name(void) const { return "CPUMeter"; }
    void checkevent(void);
    void checkResources(void);
    static const char *cpuStr(int num);

protected:
    std::vector<std::vector<float> > cputime_;
    int cpuindex_;

    void getcputime(void);

private:
    KStatList *_cpustats;
    bool _aggregate;
    kstat_ctl_t *_kc;
    kstat_t *_ksp;
};


#endif
