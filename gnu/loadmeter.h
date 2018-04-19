//
//  Copyright (c) 1994, 1995, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//  2007 by Samuel Thibault ( samuel.thibault@ens-lyon.org )
//
//  This file may be distributed under terms of the GPL
//
#ifndef loadmeter_h
#define loadmeter_h

#include "ploadmeter.h"


class LoadMeter : public PrcLoadMeter {
public:
    LoadMeter(void) : PrcLoadMeter() {}

protected:
    virtual uint64_t getCPUSpeed(void) override { return 0; }
};


#endif
