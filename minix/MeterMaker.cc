//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "MeterMaker.h"

#include "ploadmeter.h"
#include "cpumeter.h"
#include "memmeter.h"
#include "fsmeter.h"
#include "example.h"  // The example meter


MeterMaker::MeterMaker(void) {
}


std::vector<Meter *> MeterMaker::makeMeters(const ResDB &rdb) {

    // Add the example meter.  Normally you would use
    // isResourceTrue.  But example resources are not in Xdefalts
    if (rdb.getResourceOrUseDefault("example", "False") == "True")
        _meters.push_back(new ExampleMeter());

    if (rdb.isResourceTrue("load"))
        _meters.push_back(new PrcLoadMeter());

    if (rdb.isResourceTrue("cpu"))
        _meters.push_back(new CPUMeter());

    if (rdb.isResourceTrue("mem"))
        _meters.push_back(new MemMeter());

    if (rdb.isResourceTrue("filesys"))
        util::concat(_meters, FSMeterFactory().make(rdb));

    return _meters;
}
