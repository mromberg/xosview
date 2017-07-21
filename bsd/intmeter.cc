//
//  Copyright (c) 1994, 1995, 2015, 2016, 2017
//  by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  This file may be distributed under terms of the GPL
//

#include "intmeter.h"



IntMeter::IntMeter(void)
    : ComIntMeter("INTS") {
}


std::map<size_t, uint64_t> IntMeter::getStats(void) const {
    _istats.scan();
    return _istats.counts();
}
