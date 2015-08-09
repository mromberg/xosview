//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "MeterMaker.h"

#include "example.h"  // The example meter


MeterMaker::MeterMaker(XOSView *xos) : _xos(xos) {
}


std::vector<Meter *> MeterMaker::makeMeters(const ResDB &rdb) {
    std::vector<Meter *> rval;

    // Add the example meter.  Normally you would use
    // isResourceTrue.  But example resources are not in Xdefalts
    if (true || rdb.getResourceOrUseDefault("example", "False") == "True")
        rval.push_back(new ExampleMeter(_xos));

    return rval;
}
