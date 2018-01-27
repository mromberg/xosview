//
//  Copyright (c) 1994, 1995, 2002, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//  2007 by Samuel Thibault ( samuel.thibault@ens-lyon.org )
//
//  This file may be distributed under terms of the GPL
//
#include "MeterMaker.h"

#include "cpumeter.h"
#include "memmeter.h"
#include "pswapmeter.h"
#include "pagemeter.h"
#include "loadmeter.h"
#include "cfsmeter.h"
#include "example.h"  // The example meter



ComMeterMaker::mlist MeterMaker::makeMeters(const ResDB &rdb) {

    mlist meters;

    // Add the example meter.  Normally you would use
    // isResourceTrue.  But example resources are not in Xdefalts
    if (rdb.getResourceOrUseDefault("example", "False") == "True")
        meters.push_back(std::make_unique<ExampleMeter>());

    if (rdb.isResourceTrue("load"))
        meters.push_back(std::make_unique<LoadMeter>());

    if (rdb.isResourceTrue("cpu"))
        cpuFactory(rdb, meters);

    if (rdb.isResourceTrue("mem"))
        meters.push_back(std::make_unique<MemMeter>());

    if (rdb.isResourceTrue("filesys"))
        util::concat(meters, ComFSMeterFactory().make(rdb));

    if (rdb.isResourceTrue("swap"))
        meters.push_back(std::make_unique<PrcSwapMeter>());

    if (rdb.isResourceTrue("page"))
        meters.push_back(std::make_unique<PageMeter>());

    return meters;
}


void MeterMaker::cpuFactory(const ResDB &rdb, mlist &meters) const {
    size_t start = 0, end = 0;
    getRange(rdb.getResource("cpuFormat"), CPUMeter::countCPUs(), start, end);

    logDebug << "start=" << start << ", end=" << end << std::endl;

    for (size_t i = start ; i <= end ; i++)
        meters.push_back(std::make_unique<CPUMeter>(i));
}
