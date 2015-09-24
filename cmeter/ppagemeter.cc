//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "ppagemeter.h"
#include "fsutil.h"

#include <fstream>
#include <limits>

#include <unistd.h>


static const char * const VMSTATFILE = "/proc/vmstat";
static const char * const STATFILE = "/proc/stat";


PrcPageMeter::PrcPageMeter( XOSView *parent, bool useVMStat)
    : ComPageMeter(parent), _vmstat(useVMStat && util::fs::isfile(VMSTATFILE)),
      _last(getPageCount()), _pageSize(sysconf(_SC_PAGESIZE)) {
}


std::pair<float, float> PrcPageMeter::getPageRate(void) {
    _timer.stop();
    double etime = _timer.report();
    std::pair<uint64_t, uint64_t> counts(getPageCount());

    std::pair<float, float> rval((counts.first - _last.first) * _pageSize,
      (counts.second - _last.second) * _pageSize);
    rval.first /= etime;
    rval.second /= etime;

    _last = counts;

    return rval;
}


std::pair<uint64_t, uint64_t> PrcPageMeter::getPageCount(void) {
    if (_vmstat)
        return getVMStatPageCount();

    return getStatPageCount();
}


std::pair<uint64_t, uint64_t> PrcPageMeter::getStatPageCount(void) {
    std::pair<uint64_t, uint64_t> rval(0, 0);

    std::ifstream ifs(STATFILE);
    if (!ifs)
        logFatal << "could not open: " << STATFILE << std::endl;

    std::string label;
    while (ifs) {
        ifs >> label;
        if (label == "page") {
            ifs >> rval.first >> rval.second;
            break;
        }
    }

    if (!ifs)
        logFatal << "could not parse: " << STATFILE << std::endl;

    return rval;
}


std::pair<uint64_t, uint64_t> PrcPageMeter::getVMStatPageCount(void) {
    std::pair<uint64_t, uint64_t> rval(0, 0);

    std::ifstream ifs(VMSTATFILE);

    if (!ifs)
        logFatal << "could not open: " << VMSTATFILE << std::endl;

    bool foundIn = false, foundOut = false;
    std::string label;
    while (ifs) {
        ifs >> label;

        if (label == "pswpin") {
            foundIn = true;
            ifs >> rval.first;
        }
        else if (label == "pswpout") {
            foundOut = true;
            ifs >> rval.second;
        }

        if (foundOut && foundIn)
            break;

        ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    if (!ifs || !foundIn || !foundOut)
        logFatal << "failed to parse " << VMSTATFILE << std::endl;

    _timer.start();
    return rval;
}
