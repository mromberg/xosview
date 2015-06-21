//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef _INTMETER_H_
#define _INTMETER_H_

#include "bitmeter.h"

#include <vector>
#include <map>

class IntMeter : public BitMeter {
public:
    IntMeter( XOSView *parent, unsigned int cpu = 0, unsigned int cpuTot=1);
    ~IntMeter( void );

    void checkevent( void );

    void checkResources( void );

protected:
    std::vector<unsigned long> irqs_;
    std::vector<unsigned long> lastirqs_;

    unsigned int _cpu;     // which cpu are we displaying


    void getirqs( void );
    void updateirqcount( int n, bool init );
    unsigned int irqcount( void );

private:
    unsigned int _cpuTot;  // total cpus on system

    // key: line number (in proc file), value: irq number
    // lines and irqs start at 0
    static std::map<size_t,unsigned int> _irqmap;
    size_t _maxIRQ;

    // irq counts
    std::vector<unsigned long long> _last;

    std::vector<unsigned long long> readCounts(void) const;
    unsigned long long parseLine(const std::string &line) const;
    int loadIRQMap(void);
    void initUI(void);
};

#endif
