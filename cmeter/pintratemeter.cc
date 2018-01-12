//
//  Copyright (c) 2015, 2017
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "pintratemeter.h"

#include <fstream>
#include <limits>


static const char * const STATFNAME = "/proc/stat";



PrcIrqRateMeter::PrcIrqRateMeter(void)
    : ComIrqRateMeter(), _last(getIntCount()) {
}


float PrcIrqRateMeter::getIrqRate(void) {
    _timer.stop();
    double etime = _timer.report();

    float rval = 0.0;
    if (etime >= 0.001) {
        uint64_t count = getIntCount();
        if (count >= _last)
            rval = (float)(count - _last) / etime;

        logDebug << "(count, time, rate): "
                 << count - _last << ", " << etime << ", " << rval
                 << std::endl;
        _last = count;
    }

    return rval;
}


uint64_t PrcIrqRateMeter::getIntCount(void) {

    std::ifstream ifs(STATFNAME);
    if (!ifs)
        logFatal << "Could not open: " << STATFNAME << std::endl;

    // Read until we are on the intr line
    while (!ifs.eof()) {
        std::string name;
        ifs >> name;
        if (!ifs.eof() && !ifs.fail()) {
            if (name == "intr") {
                uint64_t count;
                ifs >> count;
                if (ifs.fail())
                    logFatal << "failed parsing: " << STATFNAME
                             << std::endl;
                _timer.start();
                return count;
            }
            ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }

    logProblem << "failed to find intr line in: " << STATFNAME << std::endl;
    _timer.start();                _timer.start();
    return 0;
}
