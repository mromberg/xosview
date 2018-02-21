//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef pswapmeter_h
#define pswapmeter_h

#include "cswapmeter.h"


class PrcSwapMeter : public ComSwapMeter {
public:
    PrcSwapMeter(void) : ComSwapMeter() {}

protected:
    virtual std::pair<uint64_t, uint64_t> getswapinfo(void) override;
};


#endif
