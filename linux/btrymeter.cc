//
//  Copyright (c) 1997, 2006, 2015, 2016, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "btrymeter.h"
#include "fsutil.h"
#include "strutil.h"

#include <iomanip>
#include <cmath>


static const char * const LEGEND = "CAP/USED";
static const char * const SYSDIRNAME = "/sys/class/power_supply/";



BtryMeter::BtryMeter(void)
    : FieldMeter(2, "BTRY", LEGEND),
      _stype(statType()),
      _critColor(0), _lowColor(0), _leftColor(0),
      _chargeColor(0), _fullColor(0), _noneColor(0),
      // Depricated APM and ACPI members beyond this point
      apm_battery_state(0xFF), old_apm_battery_state(0xFF),
      acpi_charge_state(-2), old_acpi_charge_state(-2) {

    logDebug << "Battery state type: " << _stype << std::endl;
}


BtryMeter::StatType BtryMeter::statType(void) const {
    // In order of preference
    if (has_sys())
        return SYS;
    if (has_acpi()) {
        logEvent << "Using depricated ACPI battery support." << std::endl;
        return ACPI;
    }
    if (has_apm()) {
        logEvent << "Using depricated APM battery support." << std::endl;
	return APM;
    }

    logProblem << "Failed to detect sys, acpi or apm battery support."
               << std::endl;
    return NONE;
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

    if (!getpwrinfo()) {
        // getting the power info failed (for some reason)
        // reset with sane defaults.
        _total = 100;
        _fields[0] = 0;
        _fields[1] = 100;
        setUsed(_fields[0], _total);
    }
}


bool BtryMeter::getpwrinfo(void) {
    // Just dispatch the call to the flavor of the day
    // battery driver.
    switch (_stype) {
    case ACPI:
        return getacpiinfo();
    case APM:
        return getapminfo();
    case SYS:
        return getsysinfo();
    default:
        logBug << "Unknown or NONE StatType: " << _stype << std::endl;
    };

    return false;
}


//----------------------------------------------------------
// **SYS methods
//----------------------------------------------------------
bool BtryMeter::has_sys(void) const {
    // If the directory is there we assume
    // the new sysfs way is supported.
    // There still may or may not be a battery
    return util::fs::isdir(SYSDIRNAME);
}


bool BtryMeter::getcapacity(const std::string &batDir,
  unsigned int &capacity) const {

    capacity = 0;

    // The star of the show (capacity).
    // Apparently the star can be a bit of a drama queen and sometimes
    // does not make an appearance.  Check for her backup files
    // charge_full and charge_now.
    const std::string capFile = batDir + "capacity";
    const std::string cfFile = batDir + "charge_full";
    const std::string cnFile = batDir + "charge_now";
    if (util::fs::isfile(capFile)) {
        if (!util::fs::readFirst(capFile, capacity)) {
            logProblem << "error reading : " << capFile << std::endl;
            return false;
        }
    }
    else if (util::fs::isfile(cfFile) && util::fs::isfile(cnFile)) {
        unsigned long charge_full = 0, charge_now = 0;
        if (!util::fs::readFirst(cfFile, charge_full)
          || !util::fs::readFirst(cnFile, charge_now)) {
            logProblem << "error reading : "
                       << cfFile << " and/or " << cnFile << std::endl;
            return false;
        }
        capacity = (100 * charge_now) / charge_full;
    }
    else {
        logProblem << "Can't find a way to read capacity." << std::endl;
        return false;
    }

    return true;
}


// More words of wisdom from the interwebs:
// current_now: uA
// voltage_now: uV
// temp: 0,125 * value = real degrees celcius
// capacity: %
// health: good, dead, Overheat, over voltage, unknown
// status: Full, Discharging, Charging
// present: 0/1
bool BtryMeter::getsysinfo(void) {

    const std::string batDir = getBatDir();
    if (batDir.empty()) // have sys but no bat dirs
        return false;

    unsigned int capacity = 0;
    if (!getcapacity(batDir, capacity))
        return false;

    std::string status;
    if (!util::fs::readFirst(batDir + "status", status)) {
        logProblem << "Error reading: " << batDir + "status\n";
    }

    // The hours left looks off when charging and nearing full charge
    std::string timeLeft;
    if (status == "Discharging") {
        const float hl = getHoursLeft(batDir);
        timeLeft = timeStr(hl);
    }

    // And... the capacity can go higher than 100 (129 for me)
    // at Full (and briefly going in bat power).  Clamp it.
    if (capacity > 100)
        capacity = 100;

    // set UI components to report our findings
    _fields[0] = capacity;
    _fields[1] = 100 - capacity;
    _total = 100;

    // I just makde these numbers up.  Perhaps
    // another resource?
    if (capacity <= 10)
        setfieldcolor(0, _critColor);
    else if (capacity <= 35)
        setfieldcolor(0, _lowColor);
    else
        setfieldcolor(0, _leftColor);

    setUsed(_fields[0], _total);

    const std::string newLegend(std::string("CAP ") + timeLeft
      + "(" + status + ")/USED");

    legend(newLegend);

    return true;
}


// Apparently the following is the current state of the documentation
// for the new /sys/ way of getting this info.  You have to get lucky
// on an internet search to find this post.  Snippet copied
// for reference.
//
// > Yves-Alexis Perez wrote:
// > Currently, sysfs interface repeats brokenness of /proc/acpi/battery
// > interface.
// > ...
// > The proper way to handle it is to check for 'energy_now' and if it is
// > present, look for 'power_now'. It should be
// > present in 2.6.30 and later kernels (remaining time
// > energy_now/power_now). If it is not present (as in 2.6.29), then you
// > should assume that 'current_now' has value in uW, and remaining time is
// > energy_now/current_now.
// > If you have 'charge_now', remaining time is charge_now/current_now
// > without any complications.
//
float BtryMeter::getHoursLeft(const std::string &batDir) const {

    const auto dir = util::fs::listdir(batDir);

    if (util::find<std::string>(dir, "energy_now")) {
        const std::string fname = batDir + "energy_now";
        unsigned long long energy_now;
        if (!util::fs::readFirst(fname, energy_now)) {
            logProblem << "error reading: " << fname << std::endl;
            return 0.0;
        }
        if (util::find<std::string>(dir, "power_now")) {
            // remaining time (energy_now/power_now)
            const std::string fname = batDir + "power_now";
            unsigned long long power_now;
            if (!util::fs::readFirst(fname, power_now)) {
                logProblem << "error reading: " << fname << std::endl;
                return 0.0;
            }
            return static_cast<double>(energy_now)
                / static_cast<double>(power_now);
        }
        else {
            // remaining time is (energy_now/current_now)
            const std::string fname = batDir + "current_now";
            unsigned long long current_now;
            if (!util::fs::readFirst(fname, current_now)) {
                logProblem << "error reading: " << fname << std::endl;
                return 0.0;
            }
            return static_cast<double>(energy_now)
                / static_cast<double>(current_now);
        }
    }
    else if (util::find<std::string>(dir, "charge_now")) {
        // remaining time is charge_now/current_now
        unsigned long long charge_now, current_now;

        if (!util::fs::readFirst(batDir + "charge_now", charge_now) ||
          !util::fs::readFirst(batDir + "current_now", current_now)) {
            logProblem << "error reading: " << batDir << "["
                       << "charge_now|current_now]" << std::endl;
            return 0.0;
        }
        return (double)charge_now / (double)current_now;
    }
    else {
        logProblem << "Can't find charge_now or energy_now in: "
                   << batDir << std::endl;
    }

    return 0.0;
}


std::string BtryMeter::timeStr(float hours) const {
    float intHours = 0;
    const float fracHours = std::modf(hours, &intHours);

    std::ostringstream mins;
    mins << std::setfill('0') << std::setw(2)
         << static_cast<unsigned int>(fracHours * 60);

    return std::to_string(intHours) + ":" + mins.str();
}


std::string BtryMeter::getBatDir(void) const {
    // create a list of all the BAT* subdirs
    const auto dir = util::fs::listdir(SYSDIRNAME);
    std::vector<std::string> bats;
    std::copy_if(dir.cbegin(), dir.cend(), std::back_inserter(bats),
      [](const auto &fn){ return fn.substr(0, 3) == "BAT"; });
    std::sort(bats.begin(), bats.end());

    if (bats.empty()) // No batteries found
        return "";

    if (bats.size() > 1)
        logEvent << "Multiple batteries found: " << bats << std::endl
                 << "Only using: " << bats.front() << " for now" << std::endl;

    return std::string(SYSDIRNAME) + bats.front() + "/";
}





//--------------------------------------------------
// Depricated APM and ACPI members beyond this point
//--------------------------------------------------
// **APM
static const char * const APMFILENAME = "/proc/apm";

// determine if /proc/apm exists and is readable
bool BtryMeter::has_apm(void) const {

    if (!util::fs::isfile(APMFILENAME)) {
        logDebug << "no APM file" << std::endl;
        return false;
    }

    logDebug << "apm: " << APMFILENAME << " exists and is a file"
             << std::endl;

    std::ifstream ifs(APMFILENAME);
    if (!ifs) {
        logDebug << "open failed on " << APMFILENAME
                 << std::endl;
        return false;
    }

    // all our tests succeeded - apm seems usable
    return true;
}


bool BtryMeter::getapminfo(void) {
    // just a tiny note here about APM states:
    // See: arch/i386/kernel/apm.c apm_get_info()

    // Arguments, with symbols from linux/apm_bios.h.  Information is
    //     from the Get Power Status (0x0a) call unless otherwise noted.
    //
    //        0) Linux driver version (this will change if format changes)
    //        1) APM BIOS Version.  Usually 1.0, 1.1 or 1.2.
    //        2) APM flags from APM Installation Check (0x00):
    //           bit 0: APM_16_BIT_SUPPORT
    //           bit 1: APM_32_BIT_SUPPORT
    //           bit 2: APM_IDLE_SLOWS_CLOCK
    //           bit 3: APM_BIOS_DISABLED
    //           bit 4: APM_BIOS_DISENGAGED
    //        3) AC line status
    //           0x00: Off-line
    //           0x01: On-line
    //           0x02: On backup power (BIOS >= 1.1 only)
    //           0xff: Unknown
    //        4) Battery status
    //           0x00: High
    //           0x01: Low
    //           0x02: Critical
    //           0x03: Charging
    //           0x04: Selected battery not present (BIOS >= 1.2 only)
    //           0xff: Unknown
    //        5) Battery flag
    //           bit 0: High
    //           bit 1: Low
    //           bit 2: Critical
    //           bit 3: Charging
    //           bit 7: No system battery
    //           0xff: Unknown
    //        6) Remaining battery life (percentage of charge):
    //           0-100: valid
    //           -1: Unknown
    //        7) Remaining battery life (time units):
    //           Number of remaining minutes or seconds
    //           -1: Unknown
    //        8) min = minutes; sec = seconds

    std::ifstream loadinfo(APMFILENAME);

    if (!loadinfo.good()) {
        logDebug << "Can not open file : " <<  APMFILENAME << std::endl;
        return false;
    }

    int battery_status = 0xff; // assume unknown as default
    std::string buff;

    loadinfo >> buff >> buff >> buff >> buff >> std::hex
             >> battery_status >> buff >> _fields[0];

    logDebug << "apm battery_status is: " << battery_status << std::endl;

    // save previous state
    // if there was no state-change - the gui won't do full redraw

    old_apm_battery_state = apm_battery_state;
    apm_battery_state = battery_status;

    // If the battery status is reported as a negative number, it means we are
    // running on AC power and no battery status is available - Report it as
    // completely empty (0). (Refer to Debian bug report #281565)
    if (_fields[0] < 0)
        _fields[0] = 0;

    _total = 100;

    if (apm_battery_state != 0xFF)
        _fields[1] = _total - _fields[0];
    else { // prevent setting it to '-1' if no batt
	_fields[0] = 0;
        _fields[1] = _total;
    }

    setUsed(_fields[0], _total);

    // if the APM state changed - we need to update the colors,
    // AND force a legend redraw - otherwise not ...
    // ... and the apm-state won't change if we don't have APM.
    if (old_apm_battery_state != apm_battery_state) {
        std::string newLegend;
	// so let's eval the apm_battery_state in some more detail.

	switch (apm_battery_state) {
	case 0: // high (e.g. over 25% on my box).
            logDebug << "battery_status HIGH" << std::endl;
            setfieldcolor(0, _leftColor);
            newLegend = "High CAP/USED";
            break;
	case 1: // low  ( e.g. under 25% on my box ).
            logDebug << "battery_status LOW" << std::endl;
            setfieldcolor(0, _lowColor);
            newLegend = "LOW avail/used";
            break;
	case 2: // critical ( less than  5% on my box ).
            logDebug << "battery_status CRITICAL" << std::endl;
            setfieldcolor(0, _critColor);
            newLegend = "Crit LOW/Used";
            break;
	case 3: // Charging.
            logDebug << "battery_status CHARGING" << std::endl;
            setfieldcolor(0, _chargeColor);
            newLegend = "AC/Charging";
            break;
	case 4: // selected batt not present.
		// no idea how this state ever could happen with APM.
            logDebug << "battery_status not present" << std::endl;
            setfieldcolor(0, _noneColor);
            newLegend = "Not Present/N.A.";
            break;
	case 255: // unknown - do nothing - maybe not APM.
            // on my system this state comes if you pull both batteries
            // ( while on AC of course :-)) ).
            logDebug << "apm battery_state not known" << std::endl;
            setfieldcolor(0, _noneColor);
            newLegend = "Unknown/N.A.";
            break;
	}

        legend(newLegend);
    }

    return true;
}


// **ACPI
static const char * const ACPIBATTERYDIR = "/proc/acpi/battery";


// determine if /proc/acpi/battery exists and is a DIR
// (XXX: too lazy -  no tests for actual readability is done)
bool BtryMeter::has_acpi(void) const {

    if (!util::fs::isdir(ACPIBATTERYDIR)) {
        logDebug << "has_acpi(): stat failed: " << ACPIBATTERYDIR
                 << std::endl;
        return false;
    }
    logDebug << "exists and is a DIR." << std::endl;

    // declare ACPI as usable
    return true;
}


// ACPI provides a lot of info,
// but munging it into something usefull is ugly
// esp. as you can have more than one battery ...

bool BtryMeter::getacpiinfo(void) {

    if (!util::fs::isdir(ACPIBATTERYDIR)) {
        logDebug << "ACPI: Cannot open directory : " <<  ACPIBATTERYDIR
                 << std::endl;
        return false;
    }

    bool found = false; // whether we found at least ONE battery

    // reset all sums
    acpi_sum_cap = 0;
    acpi_sum_remain = 0;
    acpi_sum_rate = 0;
    acpi_sum_alarm = 0;

    old_acpi_charge_state = acpi_charge_state; // save old state
    acpi_charge_state = 0; // assume charged

    const std::string abs_battery_dir = ACPIBATTERYDIR;
    for (const auto &fn : util::fs::listdir(abs_battery_dir)) {
        const std::string abs_battery_name = abs_battery_dir + "/" + fn;

        logDebug << "ACPI Batt: " << fn << std::endl;

        // still can happen that it's not present:
        if (battery_present(abs_battery_name + "/info")) {
            // ok, worth to parse out all the fields
            if (parse_battery(abs_battery_name)) {
		// sum up, clip values to get realistic on
		// full-charged batteries.
		if (battery.remaining_capacity >= battery.last_full_capacity)
                    battery.remaining_capacity = battery.last_full_capacity;

		acpi_sum_cap += battery.last_full_capacity;
		acpi_sum_remain += battery.remaining_capacity;
		acpi_sum_rate += battery.present_rate;
		acpi_sum_alarm += battery.alarm;

		// sum up charge state ...
		// works only w. signed formats
		acpi_charge_state |= battery.charging_state;

                found = true; // found at least one
            }
        }
    }

    _total = 100;

    // convert into percent vals
    // XOSDEBUG("acpi: total max=%d, remain=%d\n",acpi_sum_cap,acpi_sum_remain);

    // below alarm ?
    if (acpi_sum_alarm >= acpi_sum_remain)
        acpi_charge_state = -2;

    // if NONE of the batts is present:
    if (found) {
        _fields[0] = static_cast<float>(acpi_sum_remain)
            / static_cast<float>(acpi_sum_cap * 100.0);
    } else {
        // none of the batts is present
        // (just pull out both while on AC)
        _fields[0] = 0;
        acpi_charge_state = -3;
    }

    _fields[1] = _total - _fields[0];

    setUsed(_fields[0], _total);

    // if the ACPI state changed - we need to update the colors,
    // AND force a legend redraw - otherwise not ...
    // ... and the acpi-state won't change if we don't have acpi.

    if (old_acpi_charge_state != acpi_charge_state) {
        logDebug << "ACPI: charge_state: "
                 << "old=" << old_acpi_charge_state << ", "
                 << "now=" << acpi_charge_state << std::endl;

        std::string newLegend;
	// so let's eval the apm_battery_state in some more detail.

	switch (acpi_charge_state) {
	case 0:  // charged
            logDebug << "battery_status CHARGED" << std::endl;
            setfieldcolor(0, _fullColor);
            newLegend = "CHARGED/FULL";
            break;
	case -1: // discharging
            logDebug << "battery_status DISCHARGING" << std::endl;
            setfieldcolor(0, _leftColor);
            newLegend = "CAP/USED";
            break;
	case -2: // discharging - below alarm
            logDebug << "battery_status ALARM DISCHARGING" << std::endl;
            setfieldcolor(0, _critColor);
            newLegend = "LOW/ALARM";
            break;
	case -3: // not present
            logDebug << "battery_status NOT PRESENT" << std::endl;
            setfieldcolor(0, _noneColor);
            newLegend = "NONE/NONE";
            break;
	case 1:  // charging
            logDebug << "battery_status CHARGING" << std::endl;
            setfieldcolor(0, _chargeColor);
            newLegend = "AC/Charging";
            break;
	}

        legend(newLegend);
    }

    return true;
}


// present yes/no can change anytime !
// by adding/removing a battery
bool BtryMeter::battery_present(const std::string& filename) const {

    std::ifstream loadinfo(filename);

    while (!loadinfo.eof()) {
        std::string argname;
        std::string argval;
	loadinfo >> argname >> argval;

        logDebug << "batt ?: a=" << argname << " v=" << argval << std::endl;

	if (argname == "present:" && argval == "yes")
            return true;
    }

    logDebug << "batt " << filename << " not present" << std::endl;
    return false;
}


bool BtryMeter::parse_battery(const std::string& dirname) {
    // actually there are THREE files to check:
    // 'alarm', 'info' and 'state'
    parse_alarm(dirname);
    parse_info(dirname);

    std::ifstream loadinfo(dirname + "/state");

    while (loadinfo.good()) {
	// argname can contain spaces
        std::string argname, argval;
	std::getline(loadinfo,argname, ':');

	// argval should NOT contain blanks
        std::string inp_line;
	std::getline(loadinfo, inp_line);
        std::istringstream inp_strm(inp_line);

	inp_strm >> argval; // this ignores leading spaces

        logDebug << "state: a=" << argname << " v=" << argval << std::endl;

	if (argname == "charging state") {
            if (argval == "charged")
                battery.charging_state=0;
            else if (argval == "discharging")
                battery.charging_state=-1;
            else if (argval == "charging")
                battery.charging_state=1;
	}
	else if (argname == "last full capacity")
            battery.last_full_capacity = std::stoi(argval);
	else if (argname == "last full capacity")
            battery.last_full_capacity = std::stoi(argval);
	else if (argname == "remaining capacity")
            battery.remaining_capacity = std::stoi(argval);
    }

    return true;
}


void BtryMeter::parse_alarm(const std::string &dirname) {

    std::ifstream loadinfo(dirname + "/alarm");

    while (loadinfo.good()) {
        std::string inp_line;
	std::getline(loadinfo, inp_line);
	std::istringstream inp_strm(inp_line);
        std::string argname, argval;

	inp_strm >> argname >> argval;

        logDebug << "alarm: a=" << argname << " v=" << argval << std::endl;

	if (argname == "alarm:") {
            battery.alarm = std::stoi(argval);
            break;
	}
    }
}


void BtryMeter::parse_info(const std::string &dirname) {

    std::ifstream loadinfo(dirname + "/info");

    while (loadinfo.good()) {
        std::string argname, argval;
	std::getline(loadinfo,argname, ':');
	std::getline(loadinfo,argval);

        logDebug << "info: a=" << argname << " v=" << argval << std::endl;

	if (argname == "design capacity")
            battery.design_capacity = std::stoi(argval);
	else if (argname == "last full capacity")
            battery.last_full_capacity = std::stoi(argval);
	else if (argname == "last full capacity")
            battery.last_full_capacity = std::stoi(argval);
    }
}
