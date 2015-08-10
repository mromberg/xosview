//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef MEMMETER_H
#define MEMMETER_H

#include "fieldmetergraph.h"

#include <vector>
#include <string>


class MemMeter : public FieldMeterGraph {
public:
    MemMeter( XOSView *parent );
    ~MemMeter( void );

    std::string name( void ) const { return "MemMeter"; }
    void checkevent(void);

    void checkResources(const ResDB &rdb);

private:
    void getmeminfo( void );
};

#endif
