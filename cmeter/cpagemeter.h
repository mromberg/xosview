//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef CPAGEMETER_H
#define CPAGEMETER_H

#include "fieldmetergraph.h"



class ComPageMeter : public FieldMeterGraph {
public:
    ComPageMeter( void );
    ~ComPageMeter( void );

    std::string resName( void ) const { return "page"; }
    void checkevent( void );

    void checkResources(const ResDB &rdb);

protected:
    // (in, out) bytes/sec
    virtual std::pair<float, float> getPageRate(void) = 0;

private:
    float _maxspeed;
};


#endif
