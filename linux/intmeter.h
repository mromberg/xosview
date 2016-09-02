//
//  Copyright (c) 1994, 1995, 2006, 2015, 2016
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef INTMETER_H
#define INTMETER_H

#include "bitmeter.h"

#include <vector>
#include <map>

#include <stdint.h>



class IntMeter : public BitMeter {
public:
    // cpu=0 (cummulate meter), cpu=1 (irqs for cpu0), cpu=2 (irqs for cpu1)
    IntMeter(size_t cpu = 0);
    ~IntMeter(void);

    void checkevent(void);

    virtual std::string resName(void) const { return "int"; }
    void checkResources(const ResDB &rdb);

private:
    size_t _cpu;                      // which cpu are we displaying.
    std::map<size_t, uint64_t> _last; // counts from last event.

    void initUI(void);
    const std::map<size_t, uint64_t> &getStats(size_t cpu) const;
    std::vector<std::map<size_t, uint64_t> > readStats(void) const;
};


#endif
