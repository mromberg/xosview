//
//  Copyright (c) 2015
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//
#ifndef LOADMETER_H
#define LOADMETER_H

#include "cloadmeter.h"
#include "kstats.h"



class LoadMeter : public ComLoadMeter {
public:
    LoadMeter(XOSView *parent, kstat_ctl_t *kcp);

protected:
    virtual float getLoad(void);
    virtual uint64_t getCPUSpeed(void);

private:
    KStatList *cpulist;
    kstat_ctl_t *kc;
    kstat_t *ksp;
};


#endif
