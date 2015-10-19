//
//  Copyright (c) 1994, 1995, 2002, 2015
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


MeterMaker::MeterMaker() {
}


std::vector<Meter *>  MeterMaker::makeMeters(const ResDB &rdb) {

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

    if (rdb.isResourceTrue("filesys"))
        util::concat(_meters, ComFSMeterFactory().make(rdb));

    if (rdb.isResourceTrue("swap"))
        _meters.push_back(new PrcSwapMeter());

    if (rdb.isResourceTrue("page"))
        _meters.push_back(new PageMeter());

    return _meters;
}


void MeterMaker::cpuFactory(const ResDB &rdb) {
    size_t start = 0, end = 0;
    getRange(rdb, "cpuFormat", CPUMeter::countCPUs(), start, end);

    logDebug << "start=" << start << ", end=" << end << std::endl;

    for (size_t i = start ; i <= end ; i++)
        _meters.push_back(new CPUMeter(i));
}

void MeterMaker::getRange(const ResDB &rdb, const std::string &resource,
  size_t cpuCount, size_t &start, size_t &end) const {

    // check the *Format resource if multi-procesor system
    start = end = 0;

    if (cpuCount > 1) {
        std::string format(rdb.getResource(resource));
        logDebug << resource << ": " << format << std::endl;
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
            logProblem << "Unknown " << resource << ": " << format << ".  "
                       << "Using auto" << std::endl;
            end = cpuCount;
        }
    }
}
