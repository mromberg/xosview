//
//  Copyright (c) 1994, 1995, 2004, 2006, 2015
//  Initial port performed by Stefan Eilemann (eilemann@gmail.com)
//
//  This file may be distributed under terms of the GPL
//
#ifndef DISKMETER_H
#define DISKMETER_H

#include "fieldmetergraph.h"

class DiskMeter : public FieldMeterGraph {
public:
    DiskMeter( XOSView *parent, float max );
    ~DiskMeter( void );

    virtual std::string name( void ) const { return "DiskMeter"; }
    void checkevent( void );

    void checkResources( void );

protected:
    void getdiskinfo( void );

private:
    float maxspeed_;
};

#endif
