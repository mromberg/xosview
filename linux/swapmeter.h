//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef SWAPMETER_H
#define SWAPMETER_H


#include "fieldmetergraph.h"


class SwapMeter : public FieldMeterGraph {
public:
    SwapMeter( XOSView *parent );
    ~SwapMeter( void );

    std::string name( void ) const { return "SwapMeter"; }
    void checkevent( void );

    void checkResources(const ResDB &rdb);

protected:
    void getswapinfo( void );
};

#endif
