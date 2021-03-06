//
//  Copyright (c) 1994, 1995, 2006, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
//  Most of this code was written by Werner Fink <werner@suse.de>
//  Only small changes were made on my part (M.R.)
//
#ifndef loadmeter_h
#define loadmeter_h


#include "perfcount.h"
#include "cloadmeter.h"


class LoadMeter : public ComLoadMeter {
public:
    LoadMeter(void);

    virtual void checkResources(const ResDB &rdb) override;

protected:
    virtual float getLoad(void) override;
    virtual uint64_t getCPUSpeed(void) override;

private:
    size_t _cpus;
    float _sampRate;
    float _loadAvg;  // 1min load average
    PerfQuery _query;
};


#endif
