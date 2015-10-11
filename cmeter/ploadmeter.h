//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef PLOADMETER_H
#define PLOADMETER_H

#include "cloadmeter.h"


class PrcLoadMeter : public ComLoadMeter {
public:
    PrcLoadMeter(XOSView *parent) : ComLoadMeter(parent) {}

    virtual std::string resName(void) const { return "load"; }

protected:
    virtual float getLoad(void);
    virtual uint64_t getCPUSpeed(void);
};


#endif
