//
//  Copyright (c) 1994, 1995, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef SWAPMETER_H
#define SWAPMETER_H

#include "fieldmeterdecay.h"

class SwapMeter : public FieldMeterDecay {
public:
    SwapMeter( XOSView *parent );
    ~SwapMeter( void );

    virtual std::string name( void ) const { return "SwapMeter"; }
    void checkevent( void );
    void checkResources(void);

protected:
    void getswapinfo( void );
};

#endif
