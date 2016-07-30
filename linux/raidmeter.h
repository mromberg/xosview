//
//  Copyright (c) 1999, 2006, 2015, 2016
//  by Thomas Waldmann ( ThomasWaldmann@gmx.de )
//  based on work of Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef RAIDMETER_H
#define RAIDMETER_H

#include "bitfieldmeter.h"


class RAIDMeter : public BitFieldMeter {
public:
    RAIDMeter( int raiddev = 0 );
    ~RAIDMeter( void );

    void checkevent( void );

    virtual std::string resName(void) const { return "RAID"; }

    void checkResources(const ResDB &rdb);

    static int countRAIDs( void );

private:
    int _raiddev;
    unsigned long _doneColor, _todoColor;
};


#endif
