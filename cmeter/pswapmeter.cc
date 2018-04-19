//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "pswapmeter.h"

#include <fstream>
#include <limits>


static const char * const MEMFILENAME = "/proc/meminfo";



std::pair<uint64_t, uint64_t> PrcSwapMeter::getswapinfo(void) {
    std::ifstream meminfo(MEMFILENAME);
    if (!meminfo)
        logFatal << "Cannot open file : " << MEMFILENAME << std::endl;

    std::pair<uint64_t, uint64_t> rval(0, 0);

    // Get the info from the "standard" meminfo file.
    std::string buf;
    bool foundTotal = false, foundFree = false;
    while (!meminfo.eof()) {
        meminfo >> buf;

        if (buf == "SwapTotal:") {
            meminfo >> rval.first;
            foundTotal = true;
        }
        else if (buf == "SwapFree:") {
            meminfo >> rval.second;
            foundFree = true;
        }
        if (foundTotal && foundFree)
            break;

        meminfo.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    rval.first *= 1024;
    rval.second *= 1024;

    return rval;
}
