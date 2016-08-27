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
    size_t _irqcount;
    std::vector<uint64_t> _irqs, _lastirqs;
    std::vector<bool> _inbrs;
    std::map<int,int> _realintnum;
    IntrStats _istats;

    void getirqs( void );
    void updateirqcount( bool init = false );
};


#endif
