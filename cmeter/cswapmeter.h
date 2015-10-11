//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef CSWAPMETER_H
#define CSWAPMETER_H


#include "fieldmetergraph.h"

#include <stdint.h>


class ComSwapMeter : public FieldMeterGraph {
public:
    ComSwapMeter( XOSView *parent );
    ~ComSwapMeter( void );

    std::string resName( void ) const { return "swap"; }
    void checkevent( void );

    void checkResources(const ResDB &rdb);

protected:
    // first=total, second=free
    virtual std::pair<uint64_t, uint64_t> getswapinfo( void ) = 0;
};


#endif
