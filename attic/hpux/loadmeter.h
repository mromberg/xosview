//
//  Copyright (c) 1994, 1995, 1997, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef LOADMETER_H
#define LOADMETER_H


#include "fieldmeterdecay.h"

class LoadMeter : public FieldMeterDecay {
public:
    LoadMeter( XOSView *parent );
    ~LoadMeter( void );

    virtual std::string name( void ) const { return "LoadMeter"; }
    void checkevent( void );

    void checkResources( void );
protected:

    void getloadinfo( void );
    unsigned long procloadcol_, warnloadcol_;
private:
    int alarmThreshold;
};

#endif
