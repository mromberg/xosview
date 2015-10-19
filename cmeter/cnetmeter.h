//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef CNETMETER_H
#define CNETMETER_H

#include "fieldmetergraph.h"



class ComNetMeter : public FieldMeterGraph {
public:
    ComNetMeter(void);
    ~ComNetMeter( void );

    std::string resName( void ) const { return "net"; }
    void checkevent( void );

    void checkResources(const ResDB &rdb);

protected:
    // bytes/sec (in, out)
    virtual std::pair<float, float> getRates(void) = 0;

private:
    float _maxBandwidth;  // in bytes/sec
};


#endif
