//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef PPAGEMETER_H
#define PPAGEMETER_H

#include "cpagemeter.h"
#include "timer.h"

#include <stdint.h>


class PrcPageMeter : public ComPageMeter {
public:
    PrcPageMeter( XOSView *parent, bool useVMStat=true);

protected:
    virtual std::pair<float, float> getPageRate(void);

private:
    bool _vmstat;
    std::pair<uint64_t, uint64_t> _last;
    long _pageSize;
    Timer _timer;

    std::pair<uint64_t, uint64_t> getPageCount(void);
    std::pair<uint64_t, uint64_t> getVMStatPageCount(void);
    std::pair<uint64_t, uint64_t> getStatPageCount(void);
};


#endif
