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


class MeterMaker {
public:
    MeterMaker(void) {}

    std::vector<Meter *> makeMeters(const ResDB &rdb);
};


#endif
