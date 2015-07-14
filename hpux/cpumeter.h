//
//  Copyright (c) 1994, 1995, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef CPUMETER_H
#define CPUMETER_H

#include "fieldmetergraph.h"

class CPUMeter : public FieldMeterGraph {
public:
    CPUMeter( XOSView *parent );
    ~CPUMeter( void );

    virtual std::string name( void ) const { return "CPUMeter"; }
    void checkevent( void );

    void checkResources(void);
protected:
    float cputime_[2][5];
    int cpuindex_;

    void getcputime( void );
};

#endif
