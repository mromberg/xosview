//
//  Copyright (c) 1994, 1995, 2015, 2016
//  by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  This file may be distributed under terms of the GPL
//

#ifndef INTMETER_H
#define INTMETER_H

#include "bitmeter.h"
#include "intrstats.h"


class IntMeter : public BitMeter {
public:
    IntMeter(void);
    ~IntMeter( void );

    virtual std::string resName( void ) const { return "int"; }
    void checkevent( void );
    void checkResources(const ResDB &rdb);

private:
    std::map<size_t, uint64_t> _lastirqs;  // last sample.
    std::map<size_t, size_t> _irq2bit;     // irq -> bit index.
    IntrStats _istats;

    void updateirqcount(bool init=false);
};


#endif
