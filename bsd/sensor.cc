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
    : SensorFieldMeter(label, caption),
      _name(name.substr(0, name.find_first_of('.'))),
      _val(name.substr(name.find_first_of('.') + 1)),
      _nbr(nbr) {

    if (!high.empty()) {
        _hasHigh = true;
        if (!util::fstr(high, _high)) { // high given as number?
            _highName = high.substr(0, high.find_first_of('.'));
            _highVal = high.substr(high.find_first_of('.') + 1);
        }
    }
    if (!low.empty()) {
        _hasLow = true;
        if (!util::fstr(low, _low)) {  // low given as number?
            _lowName = low.substr(0, low.find_first_of('.'));
            _lowVal = low.substr(low.find_first_of('.') + 1);
        }
    }
}


void BSDSensor::checkResources(const ResDB &rdb) {
    SensorFieldMeter::checkResources(rdb);

    _actColor  = rdb.getColor("bsdsensorActColor");
    _highColor = rdb.getColor("bsdsensorHighColor");
    _lowColor  = rdb.getColor("bsdsensorLowColor");

    setfieldcolor(0, _actColor);
    setfieldcolor(1, rdb.getColor( "bsdsensorIdleColor"));
    setfieldcolor(2, _highColor);

    const std::string highDflt(rdb.getResourceOrUseDefault(
          "bsdsensorHighest", "0"));
    const std::string res = "bsdsensorHighest" + std::to_string(_nbr);
    _total = std::abs(std::stof(rdb.getResourceOrUseDefault(res, highDflt)));

    if (!_hasHigh)
        _high = _total;
    if (!_hasLow)
        _low = 0;

    // Get the unit.
    float unused;
    BSDGetSensor(_name, _val, unused, unit());
    updateLegend();
}


void BSDSensor::checkevent(void) {
    getsensor();
}


void BSDSensor::getsensor(void) {
    float value = 0;
    std::string emptyStr;
    BSDGetSensor(_name, _val, value, emptyStr);

    float high = _high, low = _low;
    if (!_highName.empty())
        BSDGetSensor(_highName, _highVal, high, emptyStr);
    if (!_lowName.empty())
        BSDGetSensor(_lowName, _lowVal, low, emptyStr);

    _fields[0] = value;
    checkFields(low, high);
}
