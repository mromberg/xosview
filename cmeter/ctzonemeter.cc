//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
#include "ctzonemeter.h"
#include "fsutil.h"


ComTZoneMeter::ComTZoneMeter(size_t zoneNum)
    : FieldMeterGraph(2, "TZ" + util::repr(zoneNum),
      "DEG C/USED"), _peak(100.0),
      _hotTrip(70), _critTrip(84),
      _normColor(0), _hotColor(0), _critColor(0) {
}


ComTZoneMeter::~ComTZoneMeter(void) {
}


void ComTZoneMeter::checkResources(const ResDB &rdb) {

    FieldMeterGraph::checkResources(rdb);

    // change "tzone" to real meter name
    _normColor = rdb.getColor("tzoneForeGround");
    _hotColor = rdb.getColor("tzoneHotColor");
    _critColor = rdb.getColor("tzoneCritColor");

    setfieldcolor(0, _normColor);
    setfieldcolor(1, rdb.getColor("tzoneBackground"));

    _peak = util::stof(rdb.getResource("tzonePeak"));
    _hotTrip = util::stof(rdb.getResource("tzoneHotTrip"));
    _critTrip = util::stof(rdb.getResource("tzoneCritTrip"));
}


void ComTZoneMeter::checkevent( void ) {

    // Read the temperature.
    float ctemp = getTemp();

    // set the fields
    _total = _peak;
    _fields[0] = ctemp;
    _fields[1] = _total - _fields[0];

    // and the colors
    if (ctemp >= _critTrip)
        setfieldcolor(0, _critColor);
    else if (ctemp >= _hotTrip)
        setfieldcolor(0, _hotColor);
    else
        setfieldcolor(0, _normColor);

    setUsed(ctemp, 1.0);
}
