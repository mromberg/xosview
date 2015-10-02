//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#ifndef METERMAKER_H
#define METERMAKER_H

#include "rdb.h"

#include <vector>

class Meter;
class XOSView;

class MeterMaker {
public:
    MeterMaker(XOSView *xos);

    std::vector<Meter *> makeMeters(const ResDB &rdb);

private:
    XOSView *_xos;
    std::vector<Meter *> _meters;
};


#endif