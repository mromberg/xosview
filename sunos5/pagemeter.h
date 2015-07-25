//
//  Copyright (c) 1999, 2015
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//

#ifndef PAGEMETER_H
#define PAGEMETER_H

#include "fieldmetergraph.h"
#include "kstats.h"



class PageMeter : public FieldMeterGraph {
public:
    PageMeter(XOSView *parent, kstat_ctl_t *kcp, float max);
    ~PageMeter(void);

    virtual std::string name(void) const { return "PageMeter"; }
    void checkevent(void);
    void checkResources(void);

protected:
    std::vector<std::vector<float> > pageinfo_;
    int pageindex_;
    float maxspeed_;

    void getpageinfo(void);

private:
    KStatList *cpustats;
    kstat_ctl_t *kc;
};


#endif
