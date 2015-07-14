//
//  Copyright (c) 1994, 1995, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef MeterMaker_h
#define MeterMaker_h

#include "pllist.h"

class Meter;
class XOSView;

class MeterMaker : public PLList<Meter *> {
public:
    MeterMaker(XOSView *xos);

    void makeMeters(void);

private:
    XOSView *_xos;
};

#endif
