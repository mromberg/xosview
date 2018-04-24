//
//  Copyright (c) 1994, 1995, 2006, 2015, 2016, 2017, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef intmeter_h
#define intmeter_h

#include "cintmeter.h"


class IntMeter : public ComIntMeter {
public:
    // cpu=0 (cummulate meter), cpu=1 (irqs for cpu0), cpu=2 (irqs for cpu1)
    IntMeter(size_t cpu=0);

protected:
    virtual const std::map<size_t, uint64_t> &getStats(void) override;

private:
    size_t _cpu;                      // which cpu are we displaying.

    const std::vector<std::map<size_t, uint64_t>> &readStats(void) const;
    std::string makeTitle(size_t cpu) const;
};


#endif
