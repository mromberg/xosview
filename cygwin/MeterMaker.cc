//
//  Copyright (c) 2015, 2016, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include <windows.h>

#include "MeterMaker.h"

#include "loadmeter.h"
#include "cpumeter.h"
#include "memmeter.h"
#include "fsmeter.h"
#include "pswapmeter.h"
#include "ppagemeter.h"
#include "diskmeter.h"
#include "netmeter.h"
#include "pintratemeter.h"
#include "tzonemeter.h"
#include "btrymeter.h"
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

    if (rdb.isResourceTrue("disk"))
        meters.push_back(std::make_unique<DiskMeter>());

    if (rdb.isResourceTrue("filesys"))
        util::concat(meters, FSMeterFactory().make(rdb));

    if (rdb.isResourceTrue("swap"))
        meters.push_back(std::make_unique<PrcSwapMeter>());

    if (rdb.isResourceTrue("page"))
        meters.push_back(std::make_unique<PrcPageMeter>());

    if (rdb.isResourceTrue("net"))
        meters.push_back(std::make_unique<NetMeter>());

    if (rdb.isResourceTrue("irqrate"))
        meters.push_back(std::make_unique<PrcIrqRateMeter>());

    if (rdb.isResourceTrue("battery"))
        meters.push_back(std::make_unique<BtryMeter>());

    if (rdb.isResourceTrue("tzone"))
        tzoneFactory(meters);

    return meters;
}


void MeterMaker::cpuFactory(const ResDB &rdb, mlist &meters) const {
    size_t start = 0, end = 0;
    getRange(rdb.getResource("cpuFormat"), CPUMeter::countCPUs(), start, end);

    logDebug << "start=" << start << ", end=" << end << std::endl;

    for (size_t i = start ; i <= end ; i++)
        meters.push_back(std::make_unique<CPUMeter>(i));
}


void MeterMaker::tzoneFactory(mlist &meters) const {
    const size_t nzones = TZoneMeter::count();

    if (!nzones)
        logProblem << "tzone enabled but no thermal zones found.\n";

    for (size_t i = 0 ; i < nzones ; i++)
        meters.push_back(std::make_unique<TZoneMeter>(i));
}
