//
//  Copyright (c) 1994, 1995, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef MEMMETER_H
#define MEMMETER_H

#include "fieldmeterdecay.h"

#include <sys/pstat.h>

class MemMeter : public FieldMeterDecay {
public:
    MemMeter( XOSView *parent );
    ~MemMeter( void );

    virtual std::string name( void ) const { return "MemMeter"; }
    void checkevent( void );

    void checkResources(void);

protected:
    struct pst_status *stats_;
    int _pageSize;

    void getmeminfo( void );
};

#endif
