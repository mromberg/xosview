//
//  Copyright (c) 2013, 2015, 2018
//  by Tomi Tapper ( tomi.o.tapper@student.jyu.fi )
//
//  Based on linux/btrymeter.cc:
//  Copyright (c) 1997, 2006 by Mike Romberg ( mike.romberg@noaa.gov )
//
//  This file may be distributed under terms of the GPL
//
#include "btrymeter.h"

#include "kernel.h"


BtryMeter::BtryMeter( void )
    : FieldMeter( 2, "BTRY", "CHRG/USED" ),
      _leftColor(0), _usedColor(0), _chargeColor(0), _fullColor(0),
      _lowColor(0), _critColor(0), _noneColor(0),
      _oldState(256) {
}


BtryMeter::~BtryMeter( void ) {
}


void BtryMeter::checkResources(const ResDB &rdb) {
    FieldMeter::checkResources(rdb);

    _leftColor = rdb.getColor("batteryLeftColor");
    _usedColor = rdb.getColor("batteryUsedColor");
    _chargeColor = rdb.getColor("batteryChargeColor");
    _fullColor = rdb.getColor("batteryFullColor");
    _lowColor = rdb.getColor("batteryLowColor");
    _critColor = rdb.getColor("batteryCritColor");
    _noneColor = rdb.getColor("batteryNoneColor");

    setfieldcolor(0, _leftColor);
    setfieldcolor(1, _usedColor);
}


void BtryMeter::checkevent( void ) {
    getstats();
}


void BtryMeter::getstats( void ) {
    int remaining;
    unsigned int state;

    BSDGetBatteryInfo(remaining, state);

    if (state != _oldState) {
        if (state == XOSVIEW_BATT_NONE) { // no battery present
            setfieldcolor(0, _noneColor);
            legend("NONE/NONE");
        }
        else if (state & XOSVIEW_BATT_FULL) { // full battery
            setfieldcolor(0, _fullColor);
            legend("CHRG/FULL");
        }
        else { // present, not full
            if (state & XOSVIEW_BATT_CRITICAL) // critical charge
                setfieldcolor(0, _critColor);
            else if (state & XOSVIEW_BATT_LOW) // low charge
                setfieldcolor(0, _lowColor);
            else { // above low, below full
                if (state & XOSVIEW_BATT_CHARGING) // is charging
                    setfieldcolor(0, _chargeColor);
                else
                    setfieldcolor(0, _leftColor);
            }
            // legend tells if charging or discharging
            if (state & XOSVIEW_BATT_CHARGING)
                legend("CHRG/AC");
            else
                legend("CHRG/USED");
        }
        _oldState = state;
    }

    _total = 100.0;
    _fields[0] = remaining;
    _fields[1] = _total - remaining;
    setUsed(_fields[0], _total);
}
