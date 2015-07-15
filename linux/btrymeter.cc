//
//  Copyright (c) 1997, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "btrymeter.h"
#include "xosview.h"
#include "fsutil.h"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cmath>


static const char LEGEND[] = "CAP/USED";
static const char SYSDIRNAME[] = "/sys/class/power_supply/";

BtryMeter::BtryMeter( XOSView *parent )
    : FieldMeter( parent, 2, "BTRY", LEGEND, 1, 1, 0 ), _stype(NONE) {

    _stype = statType();
    logDebug << "Battery state type: " << _stype << std::endl;

    // Depricated APM and ACPI members beyond this point
    old_apm_battery_state = apm_battery_state = 0xFF;
    old_acpi_charge_state = acpi_charge_state = -2;
}

BtryMeter::~BtryMeter( void ){
}

BtryMeter::StatType BtryMeter::statType(void) {
    // In order of preference
    if ( has_sys() )
        return SYS;
    if ( has_acpi() ) {
        logEvent << "Using depricated ACPI battery support." << std::endl;
        return ACPI;
    }
    if ( has_apm() ) {
        logEvent << "Using depricated APM battery support." << std::endl;
	return APM;
    }

    return NONE;
}

void BtryMeter::checkResources( void ){
    FieldMeter::checkResources();

    setfieldcolor( 0, parent_->getResource( "batteryLeftColor" ) );
    setfieldcolor( 1, parent_->getResource( "batteryUsedColor" ) );

    priority_ = util::stoi(parent_->getResource( "batteryPriority" ));
    setUsedFormat(parent_->getResource( "batteryUsedFormat" ) );
}

void BtryMeter::checkevent( void ){

    std::string oldLegend = legend();
    if (!getpwrinfo()) {
        // getting the power info failed (for some reason)
        // reset with sane defaults.
        total_ = 100;
        fields_[0] = 0;
        fields_[1] = 100;
        setUsed(fields_[0], total_);
    }

    if (legend() != oldLegend)
        drawLegend(parent_->g());
    drawfields(parent_->g());
}

bool BtryMeter::getpwrinfo( void ){
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
bool BtryMeter::has_sys(void) {
    // If the directory is there we assume
    // the new sysfs way is supported.
    // There still may or may not be a battery
    return util::fs::isdir(SYSDIRNAME);
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

    std::string batDir = getBatDir();
    if (batDir == "") // have sys but no bat dirs
        return false;

    std::vector<std::string> dir = util::fs::listdir(batDir);

    // The star of the show (capacity)
    unsigned int capacity;
    if (!util::fs::readFirst(batDir + "capacity", capacity)) {
        logProblem << "error reading : " << batDir + "capacity\n";
        return false;
    }

    std::string status;
    if (!util::fs::readFirst(batDir + "status", status)) {
        logProblem << "Error reading: " << batDir + "status\n";
    }

    // The hours left looks off when charging and nearing full charge
    std::string timeLeft;
    if (status == "Discharging") {
        float hl = getHoursLeft(batDir, dir);
        timeLeft = timeStr(hl);
    }

    // And... the capacity can go higher than 100 (129 for me)
    // at Full (and briefly going in bat power).  Clamp it.
    if (capacity > 100)
        capacity = 100;

    // set UI components to report our findings
    fields_[0] = capacity;
    fields_[1] = 100 - capacity;
    total_ = 100;

    // I just makde these numbers up.  Perhaps
    // another resource?
    if (capacity <= 10)
        setfieldcolor( 0, parent_->getResource("batteryCritColor"));
    else if (capacity <= 35)
        setfieldcolor( 0, parent_->getResource("batteryLowColor"));
    else
        setfieldcolor( 0, parent_->getResource("batteryLeftColor"));


    setUsed(fields_[0], total_);

    size_t oldSize = legend().size();
    std::string newLegend(std::string(std::string("CAP ") + timeLeft
        + "(" + status + ")/USED"));
    if (newLegend.size() < oldSize)
        newLegend.resize(oldSize, ' ');
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
float BtryMeter::getHoursLeft(const std::string &batDir,
  const std::vector<std::string> &dir) const {

    if (std::find(dir.begin(), dir.end(), "energy_now") != dir.end()) {
        unsigned long long energy_now;
        if (!util::fs::readFirst(batDir + "energy_now", energy_now)) {
            logProblem << "error reading: " << batDir + "energy_now\n";
            return 0.0;
        }
        if (std::find(dir.begin(), dir.end(), "power_now") != dir.end()) {
            // remaining time (energy_now/power_now)
            unsigned long long power_now;
            if (!util::fs::readFirst(batDir + "power_now", power_now)) {
                logProblem << "error reading: " << batDir + "power_now\n";
                return 0.0;
            }
            return (double)energy_now / (double)power_now;
        }
        else {
            // remaining time is (energy_now/current_now)
            unsigned long long current_now;
            if (!util::fs::readFirst(batDir + "current_now", current_now)) {
                logProblem << "error reading: " << batDir + "current_now\n";
                return 0.0;
            }
            return (double)energy_now / (double)current_now;
        }
    }
    else if (std::find(dir.begin(), dir.end(), "charge_now") != dir.end()) {
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

std::string BtryMeter::timeStr(float &hours) const {
    float intHours = 0;
    float fracHours = modff(hours, &intHours);

    std::ostringstream mins;
    mins << std::setfill('0') << std::setw(2) << (unsigned int)(fracHours * 60);

    std::string rval = util::repr(intHours) + ":" + mins.str();

    return rval;
}

std::string BtryMeter::getBatDir(void) const {

    // create a list of all the BAT* subdirs
    std::vector<std::string> bats;
    std::vector<std::string> dir = util::fs::listdir(SYSDIRNAME);
    for (size_t i = 0 ; i < dir.size() ; i++)
        if (dir[i].substr(0, 3) == "BAT")
            bats.push_back(dir[i]);
    std::sort(bats.begin(), bats.end());

    if (bats.size() == 0) // No batteries found
        return "";

    if (bats.size() > 1)
        logEvent << "Multiple batteries found: " << bats << std::endl
                 << "Only using: " << bats[0] << " for now" << std::endl;

    return std::string(SYSDIRNAME) + bats[0] + "/";
}





//--------------------------------------------------
// Depricated APM and ACPI members beyond this point
//--------------------------------------------------
// **APM
static const char APMFILENAME[] = "/proc/apm";

// determine if /proc/apm exists and is readable
bool BtryMeter::has_apm( void ){

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

bool BtryMeter::getapminfo( void ){
    std::ifstream loadinfo( APMFILENAME );

/* just a tiny note here about APM states:
   See: arch/i386/kernel/apm.c apm_get_info()

           Arguments, with symbols from linux/apm_bios.h.  Information is
           from the Get Power Status (0x0a) call unless otherwise noted.

           0) Linux driver version (this will change if format changes)
           1) APM BIOS Version.  Usually 1.0, 1.1 or 1.2.
           2) APM flags from APM Installation Check (0x00):
              bit 0: APM_16_BIT_SUPPORT
              bit 1: APM_32_BIT_SUPPORT
              bit 2: APM_IDLE_SLOWS_CLOCK
              bit 3: APM_BIOS_DISABLED
              bit 4: APM_BIOS_DISENGAGED
           3) AC line status
              0x00: Off-line
              0x01: On-line
              0x02: On backup power (BIOS >= 1.1 only)
              0xff: Unknown
           4) Battery status
              0x00: High
              0x01: Low
              0x02: Critical
              0x03: Charging
              0x04: Selected battery not present (BIOS >= 1.2 only)
              0xff: Unknown
           5) Battery flag
              bit 0: High
              bit 1: Low
              bit 2: Critical
              bit 3: Charging
              bit 7: No system battery
              0xff: Unknown
           6) Remaining battery life (percentage of charge):
              0-100: valid
              -1: Unknown
           7) Remaining battery life (time units):
              Number of remaining minutes or seconds
              -1: Unknown
           8) min = minutes; sec = seconds
*/

    if ( !loadinfo.good() ){
        logDebug << "Can not open file : " <<  APMFILENAME << std::endl;
        return false;
    }

    int battery_status=0xff; // assume unknown as default
    std::string buff;

    loadinfo >> buff >> buff >> buff >> buff >> std::hex
             >> battery_status >> buff >> fields_[0];


    // XOSDEBUG("apm battery_status is: %d\n",battery_status);

    // save previous state
    // if there was no state-change - the gui won't do full redraw

    old_apm_battery_state=apm_battery_state;
    apm_battery_state=battery_status;

    // If the battery status is reported as a negative number, it means we are
    // running on AC power and no battery status is available - Report it as
    // completely empty (0). (Refer to Debian bug report #281565)
    if (fields_[0] < 0)
        fields_[0] = 0;

    total_ = 100;

    if ( apm_battery_state != 0xFF ) {

        fields_[1] = total_ - fields_[0];

    } else { // prevent setting it to '-1' if no batt

	fields_[0] = 0; fields_[1] = total_;
    }

    setUsed (fields_[0], total_);

    /* if the APM state changed - we need to update the colors,
       AND force a legend redraw - otherwise not ...
       ... and the apm-state won't change if we don't have APM
    */
    if ( old_apm_battery_state != apm_battery_state ) {

        size_t oldLegendSize = legend().size();
        std::string newLegend;
	/* so let's eval the apm_battery_state in some more detail: */

	switch ( apm_battery_state ) {

	case 0: /* high (e.g. over 25% on my box) */
            logDebug << "battery_status HIGH" << std::endl;
            setfieldcolor( 0, parent_->getResource("batteryLeftColor"));
            newLegend = "High CAP/USED";
            break;

	case 1: /* low  ( e.g. under 25% on my box ) */
            logDebug << "battery_status LOW" << std::endl;
            setfieldcolor( 0, parent_->getResource("batteryLowColor"));
            newLegend = "LOW avail/used";
            break;

	case 2: /* critical ( less than  5% on my box ) */
            logDebug << "battery_status CRITICAL" << std::endl;
            setfieldcolor( 0, parent_->getResource("batteryCritColor"));
            newLegend = "Crit LOW/Used";
            break;

	case 3: /* Charging */
            logDebug << "battery_status CHARGING" << std::endl;
            setfieldcolor( 0, parent_->getResource("batteryChargeColor"));
            newLegend = "AC/Charging";
            break;

	case 4: /* selected batt not present */
		/* no idea how this state ever could happen with APM */
            logDebug << "battery_status not present" << std::endl;
            setfieldcolor( 0, parent_->getResource("batteryNoneColor"));
            newLegend = "Not Present/N.A.";
            break;

	case 255: /* unknown - do nothing - maybe not APM */
            // on my system this state comes if you pull both batteries
            // ( while on AC of course :-)) )
            logDebug << "apm battery_state not known" << std::endl;
            setfieldcolor( 0, parent_->getResource("batteryNoneColor"));
            newLegend = "Unknown/N.A.";
            break;
	}
        // The legend drawing code currently only clears the area
        // behind the text it is drawig.  Not the area of the old text.
        // So, for now make sure the legend is wide enough to cover all
        // cases.
        if (newLegend.size() < oldLegendSize)
            newLegend.resize(oldLegendSize, ' ');

        legend(newLegend);
    }

    return true;
}

// **ACPI
static const char ACPIBATTERYDIR[] = "/proc/acpi/battery";

// determine if /proc/acpi/battery exists and is a DIR
// (XXX: too lazy -  no tests for actual readability is done)
bool BtryMeter::has_acpi( void ){

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

bool BtryMeter::getacpiinfo( void ){

    if (!util::fs::isdir(ACPIBATTERYDIR)) {
        logDebug << "ACPI: Cannot open directory : " <<  ACPIBATTERYDIR
                 << std::endl;
        return false;
    }

    bool found = false; // whether we found at least ONE battery

    // reset all sums
    acpi_sum_cap=0;
    acpi_sum_remain=0;
    acpi_sum_rate=0;
    acpi_sum_alarm=0;

    // save old state
    old_acpi_charge_state=acpi_charge_state;

    acpi_charge_state=0; // assume charged

    std::string abs_battery_dir = ACPIBATTERYDIR;
    std::vector<std::string> dir = util::fs::listdir(ACPIBATTERYDIR);
    for (size_t di = 0 ; di < dir.size(); di++) {
        std::string abs_battery_name = abs_battery_dir + "/" + dir[di];

        logDebug << "ACPI Batt: " <<  dir[di] << std::endl;

        // still can happen that it's not present:
        if ( battery_present( abs_battery_name + "/info" ) ) {
            // ok, worth to parse out all the fields
            if ( parse_battery( abs_battery_name ) ) {

		// sum up:

		// clip values to get realistic on full-charged batteries

		if ( battery.remaining_capacity >= battery.last_full_capacity)
                    battery.remaining_capacity=battery.last_full_capacity;

		acpi_sum_cap   +=battery.last_full_capacity;
		acpi_sum_remain+=battery.remaining_capacity;
		acpi_sum_rate  +=battery.present_rate;
		acpi_sum_alarm +=battery.alarm;

		// sum up charge state ...
		// works only w. signed formats
		acpi_charge_state|=battery.charging_state;

                found = true; // found at least one
            }
        }
    }

    total_ = 100;

    // convert into percent vals
    // XOSDEBUG("acpi: total max=%d, remain=%d\n",acpi_sum_cap,acpi_sum_remain);

    // below alarm ?
    if ( acpi_sum_alarm >= acpi_sum_remain )
        acpi_charge_state=-2;

    // if NONE of the batts is present:
    if ( found ) {
        fields_[0] = (float)acpi_sum_remain/(float)acpi_sum_cap*100.0;
    } else {
        // none of the batts is present
        // (just pull out both while on AC)
        fields_[0] = 0;
        acpi_charge_state=-3;
    }

    fields_[1] = total_ - fields_[0];

    setUsed (fields_[0], total_);

    /* if the ACPI state changed - we need to update the colors,
       AND force a legend redraw - otherwise not ...
       ... and the acpi-state won't change if we don't have acpi
    */

    if ( old_acpi_charge_state != acpi_charge_state ) {
        logDebug << "ACPI: charge_state: "
                 << "old=" << old_acpi_charge_state << ", "
                 << "now=" << acpi_charge_state << std::endl;

        size_t oldLegendSize = legend().size();
        std::string newLegend;
	/* so let's eval the apm_battery_state in some more detail: */

	switch ( acpi_charge_state ) {
	case 0:  // charged
            logDebug << "battery_status CHARGED" << std::endl;
            setfieldcolor( 0, parent_->getResource("batteryFullColor"));
            newLegend = "CHARGED/FULL";
            break;
	case -1: // discharging
            logDebug << "battery_status DISCHARGING" << std::endl;
            setfieldcolor( 0, parent_->getResource("batteryLeftColor"));
            newLegend = "CAP/USED";
            break;
	case -2: // discharging - below alarm
            logDebug << "battery_status ALARM DISCHARGING" << std::endl;
            setfieldcolor( 0, parent_->getResource("batteryCritColor"));
            newLegend = "LOW/ALARM";
            break;
	case -3: // not present
            logDebug << "battery_status NOT PRESENT" << std::endl;
            setfieldcolor( 0, parent_->getResource("batteryNoneColor"));
            newLegend = "NONE/NONE";
            break;
	case 1:  // charging
            logDebug << "battery_status CHARGING" << std::endl;
            setfieldcolor( 0, parent_->getResource("batteryChargeColor"));
            newLegend = "AC/Charging";
            break;
	}
        // add spaces to clear the old text
        if (newLegend.size() < oldLegendSize)
            newLegend.resize(oldLegendSize, ' ');

        legend(newLegend);
    }

    return true;
}

// present yes/no can change anytime !
// by adding/removing a battery
bool BtryMeter::battery_present(const std::string& filename) {
    std::ifstream loadinfo( filename.c_str() );

    std::string argname;
    std::string argval;

    while ( !loadinfo.eof() ) {

	argname.clear(); argval.clear();
	loadinfo >> argname >> argval;

	//XOSDEBUG("batt ?: a=\"%s\" v=\"%s\"\n",argname.c_str(),argval.c_str() );
	if ( argname == "present:" )
            if ( argval == "yes" )
		return true;
    }
    logDebug << "batt " << filename << " not present" << std::endl;
    return false;
}

bool BtryMeter::parse_battery(const std::string& dirname) {
    // actually there are THREE files to check:
    // 'alarm', 'info' and 'state'

    std::string filename;
    std::ifstream loadinfo;
    std::istringstream inp_strm;
    std::string inp_line;

    std::string argname;
    std::string argval;

    filename = dirname + "/alarm";
    loadinfo.open(filename.c_str() );
    while ( loadinfo.good() ) {
	std::getline(loadinfo,inp_line);
	inp_strm.str(inp_line.c_str() );
	argname.clear(); argval.clear();
	inp_strm >> argname >> argval;
	//XOSDEBUG("alarm: a=\"%s\" v=\"%s\"\n",
        //    argname.c_str(),argval.c_str() );

	if ( argname == "alarm:" ) {
            battery.alarm = util::stoi(argval);
            break;
	}
    }
    loadinfo.close(); loadinfo.clear();

    filename = dirname + "/info";
    loadinfo.open(filename.c_str() );
    while ( loadinfo.good() ) {

	argname.clear();
	std::getline(loadinfo,argname,':');

	argval.clear();
	std::getline(loadinfo,argval);

	//XOSDEBUG("info: a=\"%s\" v=\"%s\"\n",argname.c_str(),argval.c_str() );

	if ( argname == "design capacity" ) {
            battery.design_capacity=util::stoi(argval);
	}
	if ( argname == "last full capacity" ) {
            battery.last_full_capacity=util::stoi(argval);
	}
	if ( argname == "last full capacity" ) {
            battery.last_full_capacity=util::stoi(argval);
	}

    }
    loadinfo.close(); loadinfo.clear(); // clear eof-bit

    filename = dirname + "/state";
    loadinfo.open(filename.c_str() );
    while ( loadinfo.good() ) {

	// argname can contain spaces
	argname.clear();
	std::getline(loadinfo,argname,':');

	// argval should NOT contain blanks
	inp_line.clear();
	std::getline(loadinfo,inp_line);
	inp_strm.clear(); inp_strm.seekg(0);
	inp_strm.str(inp_line.c_str() );

	argval.clear();
	inp_strm >> argval; // this ignores leading spaces

	//XOSDEBUG("state: a=\"%s\" v=\"%s\"\n",argname.c_str(),argval.c_str() );

	if ( argname == "charging state" ) {
            if ( argval == "charged" )
                battery.charging_state=0;
            if ( argval == "discharging" )
                battery.charging_state=-1;
            if ( argval == "charging" )
                battery.charging_state=1;
	}

	if ( argname == "last full capacity" ) {
            battery.last_full_capacity=util::stoi(argval);
	}
	if ( argname == "last full capacity" ) {
            battery.last_full_capacity=util::stoi(argval);
	}
	if ( argname == "remaining capacity" ) {
            battery.remaining_capacity=util::stoi(argval);
	}

    }

    return true;
}
