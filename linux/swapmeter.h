//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef SWAPMETER_H
#define SWAPMETER_H


#include "pswapmeter.h"


class SwapMeter : public PrcSwapMeter {
public:
    SwapMeter( XOSView *parent ) : PrcSwapMeter(parent) {}

    virtual std::string resName(void) const { return "swap"; }

protected:
    virtual std::pair<uint64_t, uint64_t> getswapinfo( void );
};


#endif
