//
//  Copyright (c) 2012, 2015, 2018 by Tomi Tapper <tomi.o.tapper@jyu.fi>
//
//  File based on linux/lmstemp.* by
//  Copyright (c) 2000, 2006 by Leopold Toetsch <lt@toetsch.at>
//
//  This file may be distributed under terms of the GPL
//
//
//

#include "sensor.h"
#include "kernel.h"

#include <cmath>



BSDSensor::BSDSensor(const std::string &name, const std::string &high,
  const std::string &low, const std::string &label,
  const std::string &caption, int nbr)
    : SensorFieldMeter(label, caption), _nbr(nbr) {

    _highName[0] = _highVal[0] = '\0';
    _lowName[0] = _lowVal[0] = '\0';
    std::string n(name);
    _name = n.substr( 0, n.find_first_of('.') );
    _val = n.substr( n.find_first_of('.') + 1 );
    if (high.size()) {
        _hasHigh = true;
        if (!util::fstr(high, _high)) { // high given as number?
            n = high;
            _highName = n.substr( 0, n.find_first_of('.') );
            _highVal = n.substr( n.find_first_of('.') + 1 );
        }
    }
    if (low.size()) {
        _hasLow = true;
        if (!util::fstr(low, _low)) {  // low given as number?
            n = low;
            _lowName = n.substr( 0, n.find_first_of('.') );
            _lowVal = n.substr( n.find_first_of('.') + 1 );
        }
    }
}


BSDSensor::~BSDSensor( void ) {
}


void BSDSensor::checkResources(const ResDB &rdb) {
    SensorFieldMeter::checkResources(rdb);

    _actColor  = rdb.getColor("bsdsensorActColor");
    _highColor = rdb.getColor("bsdsensorHighColor");
    _lowColor  = rdb.getColor("bsdsensorLowColor");

    setfieldcolor( 0, _actColor  );
    setfieldcolor( 1, rdb.getColor( "bsdsensorIdleColor" ) );
    setfieldcolor( 2, _highColor );

    std::string tmp(rdb.getResourceOrUseDefault(
          "bsdsensorHighest", "0" ));
    std::string s("bsdsensorHighest");
    s += util::repr(_nbr);
    _total = fabs(std::stof(rdb.getResourceOrUseDefault(s, tmp)));
    s = "bsdsensorUsedFormat" + util::repr(_nbr);

    if (!_hasHigh)
        _high = _total;
    if (!_hasLow)
        _low = 0;

    // Get the unit.
    float dummy;
    BSDGetSensor(_name, _val, dummy, unit());
    updateLegend();
}


void BSDSensor::checkevent( void ) {
    getsensor();
}


void BSDSensor::getsensor( void ) {
    float value, high = _high, low = _low;
    std::string emptyStr;
    BSDGetSensor(_name, _val, value, emptyStr);
    if ( _highName.size() )
        BSDGetSensor(_highName, _highVal, high, emptyStr);
    if ( _lowName.size() )
        BSDGetSensor(_lowName, _lowVal, low, emptyStr);

    _fields[0] = value;
    checkFields(low, high);
}
