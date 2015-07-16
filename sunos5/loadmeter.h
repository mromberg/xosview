//
//  Copyright (c) 2015
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//
#ifndef LOADMETER_H
#define LOADMETER_H

#include "fieldmetergraph.h"

// To keep the header in the .cc file
struct kstat_ctl;
typedef kstat_ctl kstat_ctl_t;
struct kstat;
typedef kstat kstat_t;


class LoadMeter : public FieldMeterGraph {
public:
    LoadMeter(XOSView *parent, kstat_ctl_t *kcp);
    ~LoadMeter(void);

    virtual std::string name(void) const { return "LoadMeter"; }
    void checkevent(void);

    void checkResources(void);

protected:
    void getloadinfo(void);
    unsigned long procloadcol_;
    unsigned long warnloadcol_;

private:
    int alarmThreshold;
    kstat_ctl_t *kc;
    kstat_t *ksp;
};

#endif
