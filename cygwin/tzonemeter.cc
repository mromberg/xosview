//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
#include "tzonemeter.h"



TZoneMeter::TZoneMeter(XOSView *parent, size_t zoneNum)
    : FieldMeterGraph(parent, 2, "TZ" + util::repr(zoneNum),
      "DEG C/USED", true, true, true), _peak(100.0),
      _hotTrip(70), _critTrip(84),
      _normColor(0), _hotColor(0), _critColor(0) {

    std::vector<std::string> clist(
        PerfQuery::expand("\\Thermal Zone Information(*)\\Temperature"));
    if (zoneNum >= clist.size())
        logFatal << "Invalid thermal zone index." << std::endl;

    if (!_query.add(clist[zoneNum]))
        logFatal << "failed to add: " << clist[zoneNum] << std::endl;
}


TZoneMeter::~TZoneMeter(void) {
}


void TZoneMeter::checkResources(const ResDB &rdb) {

    FieldMeterGraph::checkResources(rdb);

    // change "tzone" to real meter name
    _normColor = rdb.getColor("tzoneForeGround");
    _hotColor = rdb.getColor("tzoneHotColor");
    _critColor = rdb.getColor("tzoneCritColor");

    setfieldcolor(0, _normColor);
    setfieldcolor(1, rdb.getResource("tzoneBackground"));

    _peak = util::stof(rdb.getResource("tzonePeak"));
    _hotTrip = util::stof(rdb.getResource("tzoneHotTrip"));
    _critTrip = util::stof(rdb.getResource("tzoneCritTrip"));
    priority_ = util::stoi(rdb.getResource("tzonePriority"));
    dodecay_ = rdb.isResourceTrue("tzoneDecay");
    useGraph_ = rdb.isResourceTrue("tzoneGraph");
    setUsedFormat(rdb.getResource("tzoneUsedFormat"));
    decayUsed(rdb.isResourceTrue("tzoneUsedDecay"));
}


void TZoneMeter::checkevent( void ) {
    // Read the counter's temp value.  It is in Kelvin (I think)
    _query.query();
    double temp = _query.counters()[0].doubleVal();
    float ctemp = temp - 273.15;

    // set the fields
    total_ = _peak;
    fields_[0] = ctemp;
    fields_[1] = total_ - fields_[0];

    // and the colors
    if (ctemp >= _critTrip)
        setfieldcolor(0, _critColor);
    else if (ctemp >= _hotTrip)
        setfieldcolor(0, _hotColor);
    else
        setfieldcolor(0, _normColor);

    setUsed(ctemp, 1.0);
}

size_t TZoneMeter::count(void) {
    std::vector<std::string> clist(
        PerfQuery::expand("\\Thermal Zone Information(*)\\Temperature"));

    logDebug << "them zones: " << clist << std::endl;

    return clist.size();
}
