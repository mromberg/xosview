//
//  Copyright (c) 2015, 2016, 2017, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
#include "tzonemeter.h"
#include "fsutil.h"

static const char * const TZDIR  = "/sys/class/thermal";
static const char * const TZSDIR = "thermal_zone";


TZoneMeter::TZoneMeter(size_t zoneNum)
    : ComTZoneMeter(zoneNum) {

    _tempFName = std::string(TZDIR) + "/" + TZSDIR + util::repr(zoneNum)
        + "/" + "temp";
}


float TZoneMeter::getTemp(void) {
    try {
        // Read the temperature.  The docs say.
        // Unit: millidegree Celsius
        unsigned long long temp = 0;
        if (!util::fs::readFirst(_tempFName, temp))
            logProblem << "error reading: " << _tempFName << std::endl;
        return static_cast<float>(temp) / 1000.0;
    }
    catch (...) {
        // gnu libstdc++ has this bug:
        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53984
        // Remove this try/catch block if/when it is fixed.
        logProblem << "libstdc++ throwing un-requested exception.\n"
                   << "See: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53984"
                   << std::endl;
        return 0.0;
    }
}


size_t TZoneMeter::count(void) {
    if (util::fs::isdir(TZDIR)) {
        std::vector<std::string> flist = util::fs::listdir(TZDIR);
        size_t rval = 0;
        std::string tzsdir(TZSDIR);
        for (const auto &fn : flist)
            if (fn.substr(0, tzsdir.size()) == tzsdir)
                rval++;

        return rval;
    }
    else
        logProblem << "directory does not exist: " << TZDIR << std::endl;
    return 0;
}
