//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef ploadmeter_h
#define ploadmeter_h

#include "cloadmeter.h"


class PrcLoadMeter : public ComLoadMeter {
public:
    PrcLoadMeter(void) : ComLoadMeter() {}

protected:
    virtual float getLoad(void) override;
    virtual uint64_t getCPUSpeed(void) override;
};


#endif
