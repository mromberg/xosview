//
//  Copyright (c) 2015, 2016
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
#include "example.h"  // The example meter


MeterMaker::MeterMaker(void) {
}


std::vector<Meter *> MeterMaker::makeMeters(const ResDB &rdb) {

    // Add the example meter.  Normally you would use
    // isResourceTrue.  But example resources are not in Xdefalts
    if (rdb.getResourceOrUseDefault("example", "False") == "True")
        _meters.push_back(new ExampleMeter());

    if (rdb.isResourceTrue("load"))
        _meters.push_back(new LoadMeter());

    if (rdb.isResourceTrue("cpu"))
        cpuFactory(rdb);

    if (rdb.isResourceTrue("mem"))
        _meters.push_back(new MemMeter());

    if (rdb.isResourceTrue("disk"))
        _meters.push_back(new DiskMeter());

    if (rdb.isResourceTrue("filesys"))
        util::concat(_meters, FSMeterFactory().make(rdb));

    if (rdb.isResourceTrue("swap"))
        _meters.push_back(new PrcSwapMeter());

    if (rdb.isResourceTrue("page"))
        _meters.push_back(new PrcPageMeter());

    if (rdb.isResourceTrue("net"))
        _meters.push_back(new NetMeter());

    if (rdb.isResourceTrue("irqrate"))
        _meters.push_back(new PrcIrqRateMeter());

    if (rdb.isResourceTrue("tzone"))
        tzoneFactory();

    return _meters;
}


void MeterMaker::cpuFactory(const ResDB &rdb) {
    size_t start = 0, end = 0;
    getRange(rdb.getResource("cpuFormat"), CPUMeter::countCPUs(), start, end);

    logDebug << "start=" << start << ", end=" << end << std::endl;

    for (size_t i = start ; i <= end ; i++)
        _meters.push_back(new CPUMeter(i));
}


void MeterMaker::tzoneFactory(void) {
    size_t nzones = TZoneMeter::count();

    if (!nzones)
        logProblem << "tzone enabled but no thermal zones found.\n";

    for (size_t i = 0 ; i < nzones ; i++)
        _meters.push_back(new TZoneMeter(i));
}
