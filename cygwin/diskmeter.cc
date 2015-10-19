//
//  Copyright (c) 1999, 2006, 2015
//  by Mike Romberg (mike-romberg@comcast.net)
//
//  This file may be distributed under terms of the GPL
//
#include "diskmeter.h"




DiskMeter::DiskMeter(void)
    : ComDiskMeter() {

    if (!_query.add("\\LogicalDisk(_Total)\\Disk Read Bytes/sec")
      || !_query.add("\\LogicalDisk(_Total)\\Disk Write Bytes/sec"))
        logFatal << "failed to load counters." << std::endl;

    _query.query();
}

std::pair<double, double> DiskMeter::getRate(void) {

    std::pair<double, double> rval;

    _query.query();
    rval.first = _query.counters()[0].doubleVal();
    rval.second = _query.counters()[1].doubleVal();

    return rval;
}
