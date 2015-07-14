//
//  Copyright (c) 2015
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//
#ifndef PAGEMETER_H
#define PAGEMETER_H

#include "fieldmetergraph.h"

#include <kstat.h>

class PageMeter : public FieldMeterGraph {
public:
    PageMeter(XOSView *parent, kstat_ctl_t *kcp, float max);
    ~PageMeter(void);

    virtual std::string name(void) const { return "PageMeter"; }
    void checkevent(void);

    void checkResources(void);

protected:
    float pageinfo_[2][2];
    int pageindex_;
    float maxspeed_;

    void getpageinfo(void);

private:
    kstat_ctl_t *kc;
    kstat_t *ksps[64];	/* XXX */
    int ncpus;
};

#endif
