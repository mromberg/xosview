//
//  Copyright (c) 1999, 2015, 2018
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//

#ifndef pagemeter_h
#define pagemeter_h

#include "cpagemeter.h"
#include "kstats.h"



class PageMeter : public ComPageMeter {
public:
    PageMeter(kstat_ctl_t *kcp);

protected:
    virtual std::pair<float, float> getPageRate(void) override;

private:
    const size_t _psize;
    std::vector<std::vector<float>> _pageinfo;
    size_t _pageindex;
    KStatList *_cpustats;
    kstat_ctl_t *_kc;
};


#endif
