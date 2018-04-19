//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef ppagemeter_h
#define ppagemeter_h

#include "cpagemeter.h"



class PrcPageMeter : public ComPageMeter {
public:
    PrcPageMeter(bool useVMStat=true);

protected:
    virtual std::pair<float, float> getPageRate(void) override;

private:
    bool _vmstat;
    std::pair<uint64_t, uint64_t> _last;
    long _pageSize;

    std::pair<uint64_t, uint64_t> getPageCount(void);
    std::pair<uint64_t, uint64_t> getVMStatPageCount(void);
    std::pair<uint64_t, uint64_t> getStatPageCount(void);
};


#endif
