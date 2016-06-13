//
//  Copyright (c) 1997, 2005, 2006, 2015, 2016
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef BTRYMETER_H
#define BTRYMETER_H

#include "fieldmeter.h"

#include <string>


class BtryMeter : public FieldMeter {
public:
    BtryMeter( void );
    ~BtryMeter( void );

    std::string resName( void ) const { return "battery"; }
    void checkevent( void );

    void checkResources(const ResDB &rdb);

protected:
    bool getpwrinfo( void );

private:
    // Reading battery stats is a mess
    // SYS is the "new" way.  Keep the others
    // for a while
    enum StatType { NONE, SYS, APM, ACPI };
    StatType _stype;
    unsigned long _critColor, _lowColor, _leftColor, _chargeColor;
    unsigned long _fullColor, _noneColor;

    StatType statType(void);

    bool has_sys(void);
    bool getsysinfo(void);
    bool getcapacity(const std::string &batDir, unsigned int &capacity) const;
    float getHoursLeft(const std::string &batDir) const;

    std::string getBatDir(void) const;
    std::string timeStr(float &hours) const;

    //--------------------------------------------------
    // Depricated APM and ACPI members beyond this point
    //--------------------------------------------------
private:
    int apm_battery_state;
    int old_apm_battery_state;

    bool has_apm(void);
    bool getapminfo( void );

    int acpi_charge_state;
    int old_acpi_charge_state;
    int acpi_sum_cap;
    int acpi_sum_remain;
    int acpi_sum_rate;
    int acpi_sum_alarm;
    // some basic fields of 'info','alarm','state'
    // XXX: should be private
    struct acpi_batt {
        int alarm;              // in mWh
        int design_capacity;    // in mWh
        int last_full_capacity; // in mWh
        int charging_state; // charged=0,discharging=-1,charging=1
        int present_rate; // in mW, 0=unknown
        int remaining_capacity; // in mW
    };
    acpi_batt battery;

    bool has_acpi(void);
    bool getacpiinfo( void );
    bool battery_present(const std::string& filename);

    bool parse_battery(const std::string& filename);
};


#endif
