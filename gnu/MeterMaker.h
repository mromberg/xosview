//
//  Copyright (c) 1994, 1995, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//  2007 by Samuel Thibault ( samuel.thibault@ens-lyon.org )
//
//  This file may be distributed under terms of the GPL
//
#ifndef METERMAKER_H
#define METERMAKER_H

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
    void getRange(const std::string &resource, size_t cpuCount,
      size_t &start, size_t &end) const;
};

#endif
