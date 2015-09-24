//
//  Copyright (c) 1994, 1995, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//  2007 by Samuel Thibault ( samuel.thibault@ens-lyon.org )
//
//  This file may be distributed under terms of the GPL
//
#ifndef LOADMETER_H
#define LOADMETER_H

#include "ploadmeter.h"


class LoadMeter : public PrcLoadMeter {
public:
    LoadMeter( XOSView *parent ) : PrcLoadMeter(parent) {}

protected:
    virtual uint64_t getCPUSpeed(void) { return 0; }
};


#endif
