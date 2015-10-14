//
//  Copyright (c) 1994, 1995, 2015 by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  This file may be distributed under terms of the GPL
//

#ifndef INTMETER_H
#define INTMETER_H

#include "bitmeter.h"

#include <map>


class IntMeter : public BitMeter {
public:
    IntMeter(XOSView *parent);
    ~IntMeter( void );

    virtual std::string resName( void ) const { return "int"; }
    void checkevent( void );
    void checkResources(const ResDB &rdb);

private:
    unsigned int irqcount_;
    std::vector<uint64_t> irqs_, lastirqs_;
    std::vector<unsigned int> inbrs_;
    std::map<int,int> realintnum_;
    void getirqs( void );
    void updateirqcount( bool init = false );
};


#endif
