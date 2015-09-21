//
//  Copyright (c) 2015
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


MeterMaker::MeterMaker(XOSView *xos) : _xos(xos) {
}


std::vector<Meter *> MeterMaker::makeMeters(const ResDB &rdb) {

    // Add the example meter.  Normally you would use
    // isResourceTrue.  But example resources are not in Xdefalts
    if (rdb.getResourceOrUseDefault("example", "False") == "True")
        _meters.push_back(new ExampleMeter(_xos));

    if (rdb.isResourceTrue("load"))
        _meters.push_back(new LoadMeter(_xos));

    if (rdb.isResourceTrue("cpu"))
        cpuFactory(rdb);

    if (rdb.isResourceTrue("mem"))
        _meters.push_back(new MemMeter(_xos));

    if (rdb.isResourceTrue("disk"))
        _meters.push_back(new DiskMeter(_xos));

    if (rdb.isResourceTrue("filesys")) {
        std::vector<std::string> fs = FSMeter::mounts(rdb);
        for (size_t i = 0 ; i < fs.size() ; i++)
            _meters.push_back(new FSMeter(_xos, fs[i]));
    }

    if (rdb.isResourceTrue("swap"))
        _meters.push_back(new PrcSwapMeter(_xos));

    if (rdb.isResourceTrue("page"))
        _meters.push_back(new PrcPageMeter(_xos));

    if (rdb.isResourceTrue("net"))
        _meters.push_back(new NetMeter(_xos));

    if (rdb.isResourceTrue("irqrate"))
        _meters.push_back(new PrcIrqRateMeter(_xos));

    if (rdb.isResourceTrue("tzone"))
        tzoneFactory();

    return _meters;
}


void MeterMaker::cpuFactory(const ResDB &rdb) {
    size_t start = 0, end = 0;
    getRange(rdb.getResource("cpuFormat"), CPUMeter::countCPUs(), start, end);

    logDebug << "start=" << start << ", end=" << end << std::endl;

    for (size_t i = start ; i <= end ; i++)
        _meters.push_back(new CPUMeter(_xos, i));
}


void MeterMaker::getRange(const std::string &format,
  size_t cpuCount, size_t &start, size_t &end) const {

    // check the *Format resource if multi-procesor system
    start = end = 0;

    if (cpuCount > 1) {
        if (format == "single") // single meter for all cpus
            end = 0;
        else if (format == "all"){ // seperate but no cumulative
            start = 1;
            end = cpuCount;
        }
        else if (format == "both") // seperate + cumulative
            end = cpuCount;
        else if (format == "auto") // if(cpuCount==1) single else both
            end = cpuCount;
        else {
            logProblem << "Unknown format: " << format << ".  "
                       << "Using auto" << std::endl;
            end = cpuCount;
        }
    }
}


void MeterMaker::tzoneFactory(void) {
    size_t nzones = TZoneMeter::count();

    if (!nzones)
        logProblem << "tzone enabled but no thermal zones found.\n";

    for (size_t i = 0 ; i < nzones ; i++)
        _meters.push_back(new TZoneMeter(_xos, i));
}
