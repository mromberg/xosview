//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef PSWAPMETER_H
#define PSWAPMETER_H

#include "cswapmeter.h"


class PrcSwapMeter : public ComSwapMeter {
public:
    PrcSwapMeter( XOSView *parent ) : ComSwapMeter(parent) {}

protected:
    virtual std::pair<uint64_t, uint64_t> getswapinfo( void );
};


#endif
