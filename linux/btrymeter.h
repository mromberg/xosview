//
//  Copyright (c) 1997, 2005, 2006, 2015, 2016, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef btrymeter_h
#define btrymeter_h

#include "fieldmeter.h"



class BtryMeter : public FieldMeter {
public:
    BtryMeter(void);

    virtual std::string resName(void) const override { return "battery"; }
    virtual void checkevent(void) override;

    virtual void checkResources(const ResDB &rdb) override;

private:
    // Reading battery stats is a mess
    // SYS is the "new" way.  Keep the others
    // for a while
    enum StatType { NONE, SYS, APM, ACPI };
    StatType _stype;
    unsigned long _critColor, _lowColor, _leftColor, _chargeColor;
    unsigned long _fullColor, _noneColor;

    bool getpwrinfo(void);
    StatType statType(void) const;
    bool has_sys(void) const;
    bool getsysinfo(void);
    bool getcapacity(const std::string &batDir, unsigned int &capacity) const;
    float getHoursLeft(const std::string &batDir) const;
    std::string getBatDir(void) const;
    std::string timeStr(float hours) const;

    //--------------------------------------------------
    // Depricated APM and ACPI members beyond this point
    //--------------------------------------------------
    int apm_battery_state;
    int old_apm_battery_state;

    bool has_apm(void) const;
    bool getapminfo(void);

    int acpi_charge_state;
    int old_acpi_charge_state;
    int acpi_sum_cap;
    int acpi_sum_remain;
    int acpi_sum_rate;
    int acpi_sum_alarm;
    // some basic fields of 'info','alarm','state'
    struct acpi_batt {
        int alarm;              // in mWh
        int design_capacity;    // in mWh
        int last_full_capacity; // in mWh
        int charging_state; // charged=0,discharging=-1,charging=1
        int present_rate; // in mW, 0=unknown
        int remaining_capacity; // in mW
    };
    acpi_batt battery;

    bool getacpiinfo(void);
    bool parse_battery(const std::string &dirname);
    void parse_alarm(const std::string &dirname);
    void parse_info(const std::string &dirname);
    bool has_acpi(void) const;
    bool battery_present(const std::string& filename) const;
};


#endif
