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



ComMeterMaker::mlist MeterMaker::makeMeters(const ResDB &rdb) {

    mlist meters;

    // Add the example meter.  Normally you would use
    // isResourceTrue.  But example resources are not in Xdefalts
    if (rdb.getResourceOrUseDefault("example", "False") == "True")
        meters.push_back(std::make_unique<ExampleMeter>());

    if (rdb.isResourceTrue("load"))
        meters.push_back(std::make_unique<PrcLoadMeter>());

    if (rdb.isResourceTrue("cpu"))
        meters.push_back(std::make_unique<CPUMeter>());

    if (rdb.isResourceTrue("mem"))
        meters.push_back(std::make_unique<MemMeter>());

    if (rdb.isResourceTrue("filesys"))
        util::concat(meters, FSMeterFactory().make(rdb));

    return meters;
}
