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
    IntMeter( XOSView *parent, const char *title = "", const char *legend = "",
      int dolegends = 0, int dousedlegends = 0 );
    ~IntMeter( void );

    virtual std::string name( void ) const { return "IntMeter"; }
    void checkevent( void );
    void checkResources( void );

protected:
    void getirqs( void );
    void updateirqcount( bool init = false );

private:
    unsigned int irqcount_;
    std::vector<uint64_t> irqs_, lastirqs_;
    std::vector<unsigned int> inbrs_;
    std::map<int,int> realintnum_;
};

#endif
