//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "MeterMaker.h"

#include "cpumeter.h"
#include "memmeter.h"
#include "example.h"  // The example meter


MeterMaker::MeterMaker(XOSView *xos) : _xos(xos) {
}


void MeterMaker::makeMeters(void) {

    // Add the example meter.  Normally you would use
    // isResourceTrue.  But example resources are not in Xdefalts
    if (_xos->getResourceOrUseDefault("example", "False") == "True")
        push(new ExampleMeter(_xos));

    if (_xos->isResourceTrue("cpu"))
        cpuFactory();

    if (_xos->isResourceTrue("mem"))
        push(new MemMeter(_xos));
}


void MeterMaker::cpuFactory(void) {
    size_t start = 0, end = 0;
    getRange("cpuFormat", CPUMeter::countCPUs(), start, end);

    logDebug << "start=" << start << ", end=" << end << std::endl;

    for (size_t i = start ; i <= end ; i++)
        push(new CPUMeter(_xos, i));
}


void MeterMaker::getRange(const std::string &resource, size_t cpuCount,
  size_t &start, size_t &end) const {
    // check the *Format resource if multi-procesor system
    start = end = 0;

    if (cpuCount > 1) {
        std::string format(_xos->getResource(resource));
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
