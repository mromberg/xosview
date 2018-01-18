//
//  Copyright (c) 2014, 2015, 2018 by Tomi Tapper <tomi.o.tapper@jyu.fi>
//
//  This file may be distributed under terms of the GPL
//
//  Put code common to *BSD and Linux sensor meters here.
//

#include "sensorfieldmeter.h"

#include <iomanip>
#include <cmath>


SensorFieldMeter::SensorFieldMeter(const std::string &title,
  const std::string &legend)
    : FieldMeter(3, title, legend),
      _high(0), _low(0), _hasHigh(false), _hasLow(false),
      _actColor(0), _highColor(0), _lowColor(0), _negative(false) {

    setMetric(true);
}

SensorFieldMeter::~SensorFieldMeter( void ){
}

void SensorFieldMeter::updateLegend( void ){
    std::string lscale, tscale;
    std::ostringstream l;
    double limit = ( _negative ? _low : _high ), total;
    total = scaleValue(_total, tscale);
    if ( (!_negative && _hasHigh) || (_negative && _hasLow) ) {
        if ( ( 0.1 <= fabs(_total) && fabs(_total) < 9.95 ) ||
          ( 0.1 <= fabs(limit) && fabs(limit) < 9.95 ) ) {
            if ( _unit.size() ) {
                l << std::setprecision(1)
                  << "ACT(" << _unit << ")/" << limit << "/" << _total;
            }
            else
                l << std::setprecision(1) << "ACT/" << limit << "/" << _total;
        }
        else if ( ( 9.95 <= fabs(_total) && fabs(_total) < 10000 ) ||
          ( 9.95 <= fabs(limit) && fabs(limit) < 10000 ) ) {
            if ( _unit.size() ) {
                l << std::setprecision(0)
                  << "ACT(" << _unit << ")/" << limit << "/" << _total;
            }
            else
                l << std::setprecision(0) << "ACT/" << limit << "/" << _total;
        }
        else {
            limit = scaleValue(limit, lscale);
            if ( _unit.size() ) {
                l << std::setprecision(0)
                  << "ACT(" << _unit << ")/" << limit << lscale << "/"
                  << total << tscale;
            }
            else {
                l << std::setprecision(0)
                  << "ACT/" << limit << lscale << "/" << total << tscale;
            }
        }
    }
    else {
        if ( ( 0.1 <= fabs(_total) && fabs(_total) < 9.95 ) ||
          ( 0.1 <= fabs(limit) && fabs(limit) < 9.95 ) ) {
            if ( _unit.size() ) {
                l << std::setprecision(1)
                  << "ACT(" << _unit << ")/" << ( _negative ? "LOW" : "HIGH" )
                  << "/" << _total;
            }
            else {
                l << std::setprecision(1)
                  << "ACT/" << ( _negative ? "LOW" : "HIGH" )
                  << "/" << _total;
            }
        }
        else if ( ( 9.95 <= fabs(_total) && fabs(_total) < 10000 ) ||
          ( 9.95 <= fabs(limit) && fabs(limit) < 10000 ) ) {
            if ( _unit.size() ) {
                l << std::setprecision(0)
                  << "ACT(" << _unit << ")/" << ( _negative ? "LOW" : "HIGH" )
                  << "/" << _total;
            }
            else {
                l << std::setprecision(0)
                  << "ACT/" << ( _negative ? "LOW" : "HIGH" )
                  << "/" << _total;
            }
        }
        else {
            if ( _unit.size() ) {
                l << std::setprecision(0)
                  << "ACT(" << _unit << ")/" << ( _negative ? "LOW" : "HIGH" )
                  << "/" << _total << tscale;
            }
            else {
                l << std::setprecision(0)
                  << "ACT/" << ( _negative ? "LOW" : "HIGH" )
                  << "/" << total << tscale;
            }
        }
    }

    legend(l.str());
}

/* Check if meter needs to be flipped, or total/limits changed. */
/* Check also if alarm limit is reached. */
/* This is to be called after the values have been read. */
void SensorFieldMeter::checkFields( double low, double high ){
    // Most sensors stay at either positive or negative values. Consider the
    // actual value and alarm limits when deciding should the meter be showing
    // positive or negative scale.
    bool do_legend = false;

    if (_negative) {  // negative at previous run
        if (_fields[0] >= 0 || low > 0) { // flip to positive
            _negative = false;
            _total = fabs(_total);
            _high = ( _hasHigh ? high : _total );
            _low = ( _hasLow ? low : 0 );
            setfieldcolor( 2, _highColor );
            do_legend = true;
        }
        else {
            if (_hasLow && low != _low) {
                _low = low;
                do_legend = true;
            }
            if (_hasHigh && high != _high)
                _high = high;
        }
    }
    else {  // positive at previous run
        // flip to negative if value and either limit is < 0
        if ( _fields[0] < 0 &&
          ( (!_hasLow && !_hasHigh) || low < 0 || high < 0 ) ) {
            _negative = true;
            _total = -fabs(_total);
            _high = ( _hasHigh ? high : 0 );
            _low = ( _hasLow ? low : _total );
            setfieldcolor( 2, _lowColor );
            do_legend = true;
        }
        else {
            if (_hasHigh && high != _high) {
                _high = high;
                do_legend = true;
            }
            if (_hasLow && low != _low)
                _low = low;
        }
    }

    // change total if value or alarms won't fit
    double highest = fabs(_high);
    if ( fabs(_fields[0]) > highest )
        highest = fabs(_fields[0]);
    if ( fabs(_low) > highest )
        highest = fabs(_low);
    if ( highest > fabs(_total) ) {
        do_legend = true;
        int scale = floor(log10(highest));
        _total = ceil((highest / pow(10.0, scale)) * 1.25) * pow(10.0, scale);
        if (_negative) {
            _total = -fabs(_total);
            if (!_hasLow)
                _low = _total;
            if (!_hasHigh)
                _high = 0;
        }
        else {
            if (!_hasLow)
                _low = 0;
            if (!_hasHigh)
                _high = _total;
        }
    }
    if (do_legend)
        updateLegend();

    // check for alarms
    if (_fields[0] > _high) { // alarm: T > max
        if (fieldcolor(0) != _highColor) {
            setfieldcolor( 0, _highColor );
            do_legend = true;
        }
    }
    else if (_fields[0] < _low) { // alarm: T < min
        if (fieldcolor(0) != _lowColor) {
            setfieldcolor( 0, _lowColor );
            do_legend = true;
        }
    }
    else {
        if (fieldcolor(0) != _actColor) {
            setfieldcolor(0, _actColor);
            do_legend = true;
        }
    }

    setUsed(_fields[0], _total);
    if (_negative)
        _fields[1] = ( _fields[0] < _low ? 0 : _low - _fields[0] );
    else {
        if (_fields[0] < 0)
            _fields[0] = 0;
        _fields[1] = ( _fields[0] > _high ? 0 : _high - _fields[0] );
    }

    _fields[2] = _total - _fields[1] - _fields[0];
}
