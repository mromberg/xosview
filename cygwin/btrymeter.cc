//
//  Copyright (c) 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "btrymeter.h"

#include <windows.h>


BtryMeter::BtryMeter(void)
    : FieldMeter(2, "BTRY", "CAP/USED") {
}


void BtryMeter::checkResources(const ResDB &rdb) {
    FieldMeter::checkResources(rdb);

    _critColor = rdb.getColor("batteryCritColor");
    _lowColor = rdb.getColor("batteryLowColor");
    _leftColor = rdb.getColor("batteryLeftColor");
    _chargeColor = rdb.getColor("batteryChargeColor");
    _fullColor = rdb.getColor("batteryFullColor");
    _noneColor = rdb.getColor("batteryNoneColor");

    setfieldcolor(0, _leftColor);
    setfieldcolor(1, rdb.getColor( "batteryUsedColor"));
}


void BtryMeter::checkevent(void) {

    SYSTEM_POWER_STATUS sps;
    if (!GetSystemPowerStatus(&sps)) {
        logProblem << "GetSystemPowerStatus() failed." << std::endl;
        _total = 100;
        _fields[0] = 0;
        _fields[1] = _total;
        setUsed(_fields[0], _total);
        return;
    }

    const int batlife = sps.BatteryLifePercent;
    const unsigned acstatus = sps.ACLineStatus; // 0/1 off/on line, 255 unknown
    const bool high = sps.BatteryFlag & 1;
    const bool low = sps.BatteryFlag & 2;
    const bool crit = sps.BatteryFlag & 4;
    const bool charging = sps.BatteryFlag & 8;
    const bool nobat = sps.BatteryFlag & 128;
    const bool unknownbat = sps.BatteryFlag & 255;
    //const bool batsaver = sps.SystemStatusFlag; if bat saver is on.
    const int batlifetime = sps.BatteryFullLifeTime; // -1 if on AC or unknown
    logDebug << "Batlife   : " << batlife << "%\n"
             << "AC status : " << acstatus << "\n"
             << "high      : " << high << "\n"
             << "low       : " << low << "\n"
             << "crit      : " << crit << "\n"
             << "charging  : " << charging << "\n"
             << "nobat     : " << nobat << "\n"
             << "unknwn bat: " << unknownbat << "\n"
             << "bat life  : " << batlifetime << " secs."
             << std::endl;

    _total = 100;
    _fields[0] = nobat ? 0 : batlife;
    _fields[1] = _total;
    setUsed(_fields[0], _total);

    setLegend(crit, low, acstatus, charging, batlifetime);
}


void BtryMeter::setLegend(bool crit, bool low, bool acstatus,
  bool charging, int batlifetime) {

    if (crit)
        setfieldcolor(0, _critColor);
    else if (low)
        setfieldcolor(0, _lowColor);
    else
        setfieldcolor(0, _leftColor);

    std::string status;
    if (acstatus)
        status = charging ? "charging" : "AC";
    else
        status = charging ? "charging" : "discharging";

    const std::string timeLeft = (batlifetime == -1) ? "unknown"
        : std::to_string(batlifetime / 3600.0) + " hr";

    legend(std::string("CAP ") + timeLeft + " (" + status + ")/USED");
}
