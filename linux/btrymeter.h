//
//  Copyright (c) 1997, 2005, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef _BTRYMETER_H_
#define _BTRYMETER_H_


#include "fieldmeter.h"
#include <string>


class BtryMeter : public FieldMeter {
public:
    BtryMeter( XOSView *parent );
    ~BtryMeter( void );

    std::string name( void ) const { return "BtryMeter"; }
    void checkevent( void );

    void checkResources( void );

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



protected:
    bool getpwrinfo( void );



private:
    bool getapminfo( void );
    bool getacpiinfo( void );

    bool has_sys(void);
    bool getsysinfo(void);
    float getHoursLeft(const std::string &batDir,
      const std::vector<std::string> &dir) const;
    std::string getBatDir(void) const;

    // Reading battery stats is a mess
    // SYS is the "new" way.  Keep the others
    // for a while
    enum StatType { NONE, SYS, APM, ACPI };
    StatType _stype;

    bool battery_present(const std::string& filename);
    bool parse_battery(const std::string& filename);

    StatType statType(void);

    bool has_acpi(void);
    bool has_apm(void);

    int apm_battery_state;
    int old_apm_battery_state;

    int acpi_charge_state;
    int old_acpi_charge_state;

    int acpi_sum_cap;
    int acpi_sum_remain;
    int acpi_sum_rate;
    int acpi_sum_alarm;

    std::string timeStr(float &hours) const;
};


#endif
