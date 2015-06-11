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

class IntMeter : public BitMeter {
public:
    IntMeter( XOSView *parent, int cpu = 0);
    ~IntMeter( void );

    void checkevent( void );

    void checkResources( void );

    static float getLinuxVersion(void);
    static int countCPUs(void);

protected:
    std::vector<unsigned long> irqs_;
    std::vector<unsigned long> lastirqs_;

    int _cpu;
    bool _old;

    void getirqs( void );
    void updateirqcount( int n, bool init );
    void initirqcount( void );
};

#endif
