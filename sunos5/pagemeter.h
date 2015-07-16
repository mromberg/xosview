//
//  Copyright (c) 2015
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//
#ifndef PAGEMETER_H
#define PAGEMETER_H

#include "fieldmetergraph.h"

// To keep the header in the .cc file
struct kstat_ctl;
typedef kstat_ctl kstat_ctl_t;
struct kstat;
typedef kstat kstat_t;



class PageMeter : public FieldMeterGraph {
public:
    PageMeter(XOSView *parent, kstat_ctl_t *kcp, float max);
    ~PageMeter(void);

    virtual std::string name(void) const { return "PageMeter"; }
    void checkevent(void);

    void checkResources(void);

protected:
    std::vector<std::vector<float> > pageinfo_;
    //float pageinfo_[2][2];
    int pageindex_;
    float maxspeed_;

    void getpageinfo(void);

private:
    kstat_ctl_t *kc;
    std::vector<kstat_t *> ksps; /* XXX */
    // ksps[64];
    size_t ncpus;
};

#endif
