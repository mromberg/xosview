//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
//  Most of this code was written by Werner Fink <werner@suse.de>
//  Only small changes were made on my part (M.R.)
//
#ifndef LOADMETER_H
#define LOADMETER_H


#include "perfcount.h"
#include "cloadmeter.h"


class LoadMeter : public ComLoadMeter {
public:
    LoadMeter( XOSView *parent );

protected:
    virtual float getLoad(void);
    virtual uint64_t getCPUSpeed(void);

private:
    size_t _cpus;
    float _loadAvg;  // 1min load average
    PerfQuery _query;
};


#endif
