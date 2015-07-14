//
//  Copyright (c) 1994, 1995, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//  2007 by Samuel Thibault ( samuel.thibault@ens-lyon.org )
//
//  This file may be distributed under terms of the GPL
//
#ifndef MEMMETER_H
#define MEMMETER_H

#include "fieldmetergraph.h"

extern "C" {
#include <mach/vm_statistics.h>
}

class MemMeter : public FieldMeterGraph {
public:
    MemMeter( XOSView *parent );
    ~MemMeter( void );

    virtual std::string name( void ) const { return "MemMeter"; }
    void checkevent( void );

    void checkResources( void );

protected:
    void getmeminfo( void );

private:
    struct vm_statistics vmstats;
};

#endif
