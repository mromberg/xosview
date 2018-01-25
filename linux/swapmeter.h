//
//  Copyright (c) 1994, 1995, 2006, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef swapmeter_h
#define swapmeter_h


#include "pswapmeter.h"


class SwapMeter : public PrcSwapMeter {
public:
    SwapMeter(void) : PrcSwapMeter() {}

    virtual std::string resName(void) const override { return "swap"; }

protected:
    virtual std::pair<uint64_t, uint64_t> getswapinfo(void) override;
};


#endif
