//
//  Copyright (c) 2015
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//
#ifndef MEMMETER_H
#define MEMMETER_H

#include "fieldmetergraph.h"

#include <kstat.h>

class MemMeter : public FieldMeterGraph {
public:
    MemMeter(XOSView *parent, kstat_ctl_t *kcp);
    ~MemMeter(void);

    virtual std::string name(void) const { return "MemMeter"; }
    void checkevent( void );

    void checkResources(void);

protected:
    //  struct pst_status *stats_;
    int _pageSize;

    void getmeminfo( void );

private:
    kstat_ctl_t *kc;
    kstat_t *ksp;
};

#endif
