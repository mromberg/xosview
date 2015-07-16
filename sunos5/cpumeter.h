//
//  Copyright (c) 2015
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//

#ifndef CPUMETER_H
#define CPUMETER_H

#include "fieldmetergraph.h"

// To keep the header in the .cc file
struct kstat_ctl;
typedef kstat_ctl kstat_ctl_t;
struct kstat;
typedef kstat kstat_t;



class CPUMeter : public FieldMeterGraph {
public:
    CPUMeter(XOSView *parent, kstat_ctl_t *kcp, int cpuid = 0);
    ~CPUMeter(void);

    virtual std::string name(void) const { return "CPUMeter"; }
    void checkevent(void);

    void checkResources(void);

    static int countCPUs(kstat_ctl_t *kc);
    static std::string cpuStr(int num);

protected:
    std::vector<std::vector<float> > cputime_;
    int cpuindex_;

    void getcputime(void);

private:
    kstat_ctl_t *kc;
    kstat_t *ksp;
};

#endif
