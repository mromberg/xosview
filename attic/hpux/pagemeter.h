//
//  Copyright (c) 1997, 2015
//  by Mike Romberg (mike-romberg@comcast.net)
//
//  This file may be distributed under terms of the GPL
//
#ifndef PAGEMETER_H
#define PAGEMETER_H

#include "fieldmeterdecay.h"

class PageMeter : public FieldMeterDecay {
public:
    PageMeter( XOSView *parent, float max );
    ~PageMeter( void );

    virtual std::string name( void ) const { return "PageMeter"; }
    void checkevent( void );

    void checkResources( void );
protected:
    float pageinfo_[2][2];
    int pageindex_;
    float maxspeed_;

    void getpageinfo( void );
};

#endif
