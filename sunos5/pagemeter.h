//
//  Copyright (c) 1999, 2015
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//

#ifndef PAGEMETER_H
#define PAGEMETER_H

#include "cpagemeter.h"
#include "kstats.h"



class PageMeter : public ComPageMeter {
public:
    PageMeter(kstat_ctl_t *kcp);

protected:
    virtual std::pair<float, float> getPageRate(void);

private:
    const size_t _psize;
    std::vector<std::vector<float> > pageinfo_;
    int pageindex_;
    KStatList *cpustats;
    kstat_ctl_t *kc;

    void getpageinfo(void);
};


#endif
