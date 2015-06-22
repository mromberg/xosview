//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef INTMETER_H
#define INTMETER_H

#include "bitmeter.h"

#include <vector>
#include <map>

class IntMeter : public BitMeter {
public:
    // cpu=0 (cummulate meter), cpu=1 (irqs for cpu0), cpu=2 (irqs for cpu1)
    IntMeter( XOSView *parent, unsigned int cpu = 0, unsigned int cpuTot=1);
    ~IntMeter( void );

    void checkevent( void );

    void checkResources( void );

private:
    unsigned int _cpu;     // which cpu are we displaying
    unsigned int _cpuTot;  // total cpus on system

    // key: line number (in proc file), value: irq number
    // lines and irqs start at 0
    static std::map<size_t,unsigned int> _irqmap;
    size_t _maxIRQ;  // max irq number in the above map

    // irq counts last done for this _cpu
    std::vector<unsigned long long> _last;

    unsigned int irqcount(void);
    std::vector<unsigned long long> readCounts(void) const;
    unsigned long long parseLine(const std::string &line) const;
    int loadIRQMap(void);
    void initUI(void);
};

#endif
