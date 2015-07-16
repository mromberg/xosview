//
//  Copyright (c) 2015
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//
#ifndef MEMMETER_H
#define MEMMETER_H

#include "fieldmetergraph.h"


// To keep the header in the .cc file
struct kstat_ctl;
typedef kstat_ctl kstat_ctl_t;
struct kstat;
typedef kstat kstat_t;



class MemMeter : public FieldMeterGraph {
public:
    MemMeter(XOSView *parent, kstat_ctl_t *kcp);
    ~MemMeter(void);

    virtual std::string name(void) const { return "MemMeter"; }
    void checkevent( void );

    void checkResources(void);

protected:
    int _pageSize;

    void getmeminfo( void );

private:
    kstat_ctl_t *kc;
    kstat_t *ksp;
};

#endif
