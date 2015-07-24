//
//  Copyright (c) 2015
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//
#ifndef LOADMETER_H
#define LOADMETER_H

#include "fieldmetergraph.h"
#include "kstats.h"



class LoadMeter : public FieldMeterGraph {
public:
    LoadMeter(XOSView *parent, kstat_ctl_t *kcp);
    ~LoadMeter(void);

    virtual std::string name(void) const { return "LoadMeter"; }
    void checkevent(void);
    void checkResources(void);

protected:
    void getloadinfo(void);
    void getspeedinfo(void);

private:
    unsigned long procloadcol, warnloadcol, critloadcol;
    unsigned int warnThreshold, critThreshold;
    unsigned int old_cpu_speed, cur_cpu_speed;
    int lastalarmstate;
    bool do_cpu_speed;
    KStatList *cpulist;
    kstat_ctl_t *kc;
    kstat_t *ksp;
};

#endif
