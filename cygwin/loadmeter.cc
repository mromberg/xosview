//
//  Copyright (c) 1994, 1995, 2006, 2008, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "loadmeter.h"
#include "cpumeter.h"   // for countCPUs()

#include <cmath>


LoadMeter::LoadMeter( void )
    : ComLoadMeter(), _cpus(CPUMeter::countCPUs()),
      _sampRate(5), _loadAvg(0.0) {

    if (!_query.add("\\Processor(_Total)\\% Processor Time") ||
      !_query.add("\\System\\Processor Queue Length") ||
      !_query.add("\\Processor Information(_Total)\\Processor Frequency"))
        logFatal << "failed to add processor queue counters." << std::endl;

    _query.query();
}


void LoadMeter::checkResources(const ResDB &rdb) {
    ComLoadMeter::checkResources(rdb);
    _sampRate = util::stof(rdb.getResource("loadPriority"))
        / util::stof(rdb.getResource("samplesPerSec"));
}


float LoadMeter::getLoad(void) {
    _query.query();

    double usedCPU = _query.counters()[0].doubleVal();
    long queueLen = _query.counters()[1].longVal();

    // rough guess since winders does not have this
    float load = queueLen + usedCPU / 100.0 * _cpus;

    // calculate the exponential moving average
    _loadAvg = load + exp(-1.0 * _sampRate / 60.0) * (_loadAvg - load);

    logDebug << "cpu: " << usedCPU << ", queue: " << queueLen << std::endl;
    logDebug << "load: " << load << ", AVG: " << _loadAvg << std::endl;

    return _loadAvg;
}


uint64_t LoadMeter::getCPUSpeed(void) {
    uint64_t rval = _query.counters()[2].doubleVal() * 1000000;
    logDebug << "freq: " <<  rval << std::endl;
    return rval;
}
