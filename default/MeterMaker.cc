//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "MeterMaker.h"

#include "example.h"  // The example meter


ComMeterMaker::mlist MeterMaker::makeMeters(const ResDB &rdb) {
    mlist rval;

    // Add the example meter.  Normally you would use
    // isResourceTrue.  But example resources are not in Xdefalts
    if (true || rdb.getResourceOrUseDefault("example", "False") == "True")
        rval.push_back(std::make_unique<ExampleMeter>());

    return rval;
}
