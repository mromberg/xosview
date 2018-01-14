//
//  Copyright (c) 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "btrymeter.h"


BtryMeter::BtryMeter(void)
    : FieldMeter(2, "BTRY", "CAP/USED") {

    total_ = 100;
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

void BtryMeter::checkevent(void){

    SYSTEM_POWER_STATUS sps;
    if (!GetSystemPowerStatus(&sps))
        logProblem << "GetSystemPowerStatus() failed." << std::endl;
    else {
        int batlife = sps.BatteryLifePercent;
        unsigned acstatus = sps.ACLineStatus; // 0/1 off/on line, 255 unknown
        bool high = sps.BatteryFlag & 1;
        bool low = sps.BatteryFlag & 2;
        bool crit = sps.BatteryFlag & 4;
        bool charging = sps.BatteryFlag & 8;
        bool nobat = sps.BatteryFlag & 128;
        bool unknownbat = sps.BatteryFlag & 255;
        //bool batsaver = sps.SystemStatusFlag; if bat saver is on.
        int batlifetime = sps.BatteryFullLifeTime; // -1 if on AC or unknown
        logDebug << "Bat life %: " << batlife << "\n"
                 << "AC status: " << acstatus << "\n"
                 << "high: " << high << "\n"
                 << "low: " << low << "\n"
                 << "crit: " << crit << "\n"
                 << "charging: " << charging << "\n"
                 << "nobat: " << nobat << "\n"
                 << "unknow bat: " << unknownbat << "\n"
                 << "bat life: " << batlifetime << " secs.\n";

        fields_[0] = nobat ? 0 : batlife;
        fields_[1] = total_;
        setUsed(fields_[0], total_);

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

        std::string timeLeft = "unknown";
        if (batlifetime != -1)
            timeLeft = util::repr(batlifetime / 3600.0) + " hr";
        std::string newLegend(std::string(std::string("CAP ") + timeLeft
            + "(" + status + ")/USED"));
        legend(newLegend);
    }
}
