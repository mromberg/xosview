//
//  Copyright (c) 2015, 2018
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//
#ifndef loadmeter_h
#define loadmeter_h

#include "cloadmeter.h"
#include "kstats.h"



class LoadMeter : public ComLoadMeter {
public:
    LoadMeter(kstat_ctl_t *kcp);

protected:
    virtual float getLoad(void) override;
    virtual uint64_t getCPUSpeed(void) override;

private:
    KStatList *_cpulist;
    kstat_ctl_t *_kc;
    kstat_t *_ksp;

    KStatList &cpulist(void) const { return *_cpulist; }
};


#endif
