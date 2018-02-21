//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "pnetmeter.h"

#include <fstream>
#include <limits>


static const char * const NETFILENAME = "/proc/net/dev";



PrcNetMeter::PrcNetMeter(void)
    : ComNetMeter(), _last(getStats()) {
}


std::pair<float, float> PrcNetMeter::getRates(void) {
    IntervalTimerStop();
    const double etime = IntervalTimeInSecs();
    const auto counts = getStats();

    std::pair<float, float> rval((counts.first - _last.first) / etime,
      (counts.second - _last.second) / etime);
    _last = counts;

    return rval;
}


std::pair<uint64_t, uint64_t> PrcNetMeter::getStats(void) {
    // Returns total bytes in/out
    std::ifstream ifs(NETFILENAME);
    if (!ifs)
        logFatal << "can not open: " << NETFILENAME << std::endl;

    // toss first two lines (captions)
    ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if (!ifs)
        logFatal << "error reading: " << NETFILENAME << std::endl;

    // each remaining line is stats for an interface.
    uint64_t read=0, write=0; // totals
    while(!ifs.eof()) {
        std::string buf;
        // The caption says these are in bytes
        uint64_t receive=0, trans=0;
        ifs >> buf;
        if (buf == "lo:") { // skip the loopback (it is crazy fast
            ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        if (ifs.eof()) // yea we will really hit EOF here
            break;
        ifs >> receive;

        // The transmit is the eighth one down the row
        for (size_t i = 0 ; i < 7 ; i++)
            ifs >> trans;
        ifs >> trans;

        // don't need the rest of the line
        ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (!ifs)
            logFatal << "error reading2: " << NETFILENAME << std::endl;

        read += receive;
        write += trans;
    }

    IntervalTimerStart();
    return std::make_pair(read, write);
}
