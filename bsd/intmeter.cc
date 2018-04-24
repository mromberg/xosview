//
//  Copyright (c) 1994, 1995, 2015, 2016, 2017, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "intmeter.h"



IntMeter::IntMeter(void)
    : ComIntMeter("INTS") {
}


const std::map<size_t, uint64_t> &IntMeter::getStats(void) {
    _istats.scan();
    return _istats.counts();
}
