//
//  Copyright (c) 2015
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
    // Read the temperature.  The docs say.
    // Unit: millidegree Celsius
    unsigned long long temp = 0;
    if (!util::fs::readFirst(_tempFName, temp))
        logFatal << "error reading: " << _tempFName << std::endl;
    return static_cast<float>(temp) / 1000.0;
}


size_t TZoneMeter::count(void) {
    if (util::fs::isdir(TZDIR)) {
        std::vector<std::string> flist = util::fs::listdir(TZDIR);
        size_t rval = 0;
        std::string tzsdir(TZSDIR);
        for (size_t i = 0 ; i < flist.size() ; i++)
            if (flist[i].substr(0, tzsdir.size()) == tzsdir)
                rval++;

        return rval;
    }
    else
        logProblem << "directory does not exist: " << TZDIR << std::endl;
    return 0;
}
