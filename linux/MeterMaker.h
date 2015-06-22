//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef _MeterMaker_h
#define _MeterMaker_h

#include "pllist.h"

#include <string>


class Meter;
class XOSView;

class MeterMaker : public PLList<Meter *> {
public:
    MeterMaker(XOSView *xos);

    void makeMeters(void);

private:
    XOSView *_xos;

    void cpuFactory(void);
    void serialFactory(void);
    void intFactory(void);
    void lmsTempFactory(void);
    void getRange(const std::string &resource, size_t cpuCount,
      size_t &start, size_t &end) const;
};

#endif
