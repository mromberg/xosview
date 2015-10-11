//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
#include "tzonemeter.h"



TZoneMeter::TZoneMeter(XOSView *parent, size_t zoneNum)
    : ComTZoneMeter(parent, zoneNum) {

    std::vector<std::string> clist(
        PerfQuery::expand("\\Thermal Zone Information(*)\\Temperature"));
    if (zoneNum >= clist.size())
        logFatal << "Invalid thermal zone index." << std::endl;

    if (!_query.add(clist[zoneNum]))
        logFatal << "failed to add: " << clist[zoneNum] << std::endl;
}


float TZoneMeter::getTemp(void) {
    _query.query();
    double temp = _query.counters()[0].doubleVal();
    return temp - 273.15;
}


size_t TZoneMeter::count(void) {
    std::vector<std::string> clist(
        PerfQuery::expand("\\Thermal Zone Information(*)\\Temperature"));

    logDebug << "them zones: " << clist << std::endl;

    return clist.size();
}
