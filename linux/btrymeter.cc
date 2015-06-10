//
//  Copyright (c) 1997, 2006, 2015 by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: btrymeter.cc,v 1.11 2006/02/18 05:23:27 romberg Exp $
//
#include "btrymeter.h"
#include "xosview.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

extern int errno;


static const char APMFILENAME[] = "/proc/apm";
static const char ACPIBATTERYDIR[] = "/proc/acpi/battery";

BtryMeter::BtryMeter( XOSView *parent )
  : FieldMeter( parent, 2, "BTRY", "AVAIL/USED", 1, 1, 0 ){

  // find out ONCE whether to use ACPI or APM
  use_acpi = use_apm = false;
  if ( has_apm() ) {
	use_apm=true; use_acpi=false;
  }
  if ( has_acpi() ) {
	use_acpi=true; use_apm=false;
  }

  old_apm_battery_state = apm_battery_state = 0xFF;
  old_acpi_charge_state = acpi_charge_state = -2;

}

BtryMeter::~BtryMeter( void ){
}

// determine if /proc/apm exists and is readable
bool BtryMeter::has_apm( void ){

  struct stat stbuf;
  int fd;

  if ( stat(APMFILENAME, &stbuf) != 0 ) {
     XOSDEBUG("APM: stat failed: %d - not APM ?\n",errno);
     return false;
  }
  if ( S_ISREG(stbuf.st_mode) ) {
     XOSDEBUG("apm: %s exists and is a file\n",APMFILENAME);
  } else {
     XOSDEBUG("no APM file\n");
     return false;
  }
  fd=open(APMFILENAME,O_RDONLY);
  if ( fd < 0 ) {
     XOSDEBUG("open failed on %s: with errno=%d\n",APMFILENAME,errno);
     return false;
  } else
     close(fd); // good enough ...

  // all our tests succeeded - apm seems usable
  return true;

}

// determine if /proc/acpi/battery exists and is a DIR
// (XXX: too lazy -  no tests for actual readability is done)
bool BtryMeter::has_acpi( void ){

  struct stat stbuf;

  if ( stat(ACPIBATTERYDIR, &stbuf) != 0 ) {
     XOSDEBUG("has_acpi(): stat failed: %d\n",errno);
     return false;
  }
  if ( S_ISDIR(stbuf.st_mode) ) {
     XOSDEBUG("exists and is a DIR.\n");
  } else {
     XOSDEBUG("no ACPI dir\n");
     return false;
  }

  // declare ACPI as usable
  return true;

}


void BtryMeter::checkResources( void ){
  FieldMeter::checkResources();

  setfieldcolor( 0, parent_->getResource( "batteryLeftColor" ) );
  setfieldcolor( 1, parent_->getResource( "batteryUsedColor" ) );

  priority_ = util::stoi(parent_->getResource( "batteryPriority" ));
  setUsedFormat(parent_->getResource( "batteryUsedFormat" ) );
}

void BtryMeter::checkevent( void ){

  getpwrinfo();


  /* if the APM state changed - we need to update the colors,
     AND force a legend redraw - otherwise not ...
     ... and the apm-state won't change if we don't have APM
  */
  if ( old_apm_battery_state != apm_battery_state ) {

	/* so let's eval the apm_battery_state in some more detail: */

	switch ( apm_battery_state ) {

	case 0: /* high (e.g. over 25% on my box) */
		XOSDEBUG("battery_status HIGH\n");
		setfieldcolor( 0, parent_->getResource("batteryLeftColor"));
		legend("High AVAIL/USED");
		break;

	case 1: /* low  ( e.g. under 25% on my box ) */
		XOSDEBUG("battery_status LOW\n");
		setfieldcolor( 0, parent_->getResource("batteryLowColor"));
		legend("LOW avail/used");
		break;

	case 2: /* critical ( less than  5% on my box ) */
		XOSDEBUG("battery_status CRITICAL\n");
		setfieldcolor( 0, parent_->getResource("batteryCritColor"));
		legend( "Crit LOW/Used");
		break;

	case 3: /* Charging */
		XOSDEBUG("battery_status CHARGING\n");
		setfieldcolor( 0, parent_->getResource("batteryChargeColor"));
		legend( "AC/Charging");
		break;

	case 4: /* selected batt not present */
		/* no idea how this state ever could happen with APM */
		XOSDEBUG("battery_status not present\n");
		setfieldcolor( 0, parent_->getResource("batteryNoneColor"));
		legend( "Not Present/N.A.");
	 	break;

	case 255: /* unknown - do nothing - maybe not APM */
		// on my system this state comes if you pull both batteries
		// ( while on AC of course :-)) )
		XOSDEBUG("apm battery_state not known\n");
		setfieldcolor( 0, parent_->getResource("batteryNoneColor"));
		legend( "Unknown/N.A.");
		break;
	}

  	drawfields();
	// force redraw of whole widget
	// (no other idea how to update the legend/labels)
	parent_->reallydraw();

   }


  /* if the ACPI state changed - we need to update the colors,
     AND force a legend redraw - otherwise not ...
     ... and the acpi-state won't change if we don't have acpi
  */

  if ( old_acpi_charge_state != acpi_charge_state ) {

  	XOSDEBUG("ACPI: charge_state: old=%d, now=%d\n",old_acpi_charge_state,acpi_charge_state);

	/* so let's eval the apm_battery_state in some more detail: */

	switch ( acpi_charge_state ) {
	case 0:  // charged
		XOSDEBUG("battery_status CHARGED\n");
		setfieldcolor( 0, parent_->getResource("batteryFullColor"));
		legend( "CHARGED/FULL");
		break;
	case -1: // discharging
		XOSDEBUG("battery_status DISCHARGING\n");
		setfieldcolor( 0, parent_->getResource("batteryLeftColor"));
		legend( "AVAIL/USED");
		break;
	case -2: // discharging - below alarm
		XOSDEBUG("battery_status ALARM DISCHARGING\n");
		setfieldcolor( 0, parent_->getResource("batteryCritColor"));
		legend( "LOW/ALARM");
		break;
	case -3: // not present
		XOSDEBUG("battery_status NOT PRESENT\n");
		setfieldcolor( 0, parent_->getResource("batteryNoneColor"));
		legend( "NONE/NONE");
		break;
	case 1:  // charging
		XOSDEBUG("battery_status CHARGING\n");
		setfieldcolor( 0, parent_->getResource("batteryChargeColor"));
		legend( "AC/Charging");
		break;
	}
  	drawfields();
	parent_->reallydraw();
   }

   drawfields();
}


void BtryMeter::getpwrinfo( void ){

  if ( use_acpi ) {
	getacpiinfo(); return;
  }
  if ( use_apm ) {
	getapminfo(); return;
  }

  // We can hit this spot in any of two cases:
  // - We have an ACPI system and the battery is removed
  // - We have neither ACPI nor APM in the system
  // We report an empty battery (i.e., we are running off AC power) instead of
  // original behavior of just exiting the program.
  // (Refer to Debian bug report #281565)
  total_ = 100;
  fields_[0] = 0;
  fields_[1] = 100;
  setUsed(fields_[0], total_);
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
    XOSDEBUG("Can not open file : %s\n", APMFILENAME);
    return false;
  }

  int battery_status=0xff; // assume unknown as default
  char buff[256];

  loadinfo >> buff >> buff >> buff >> buff >> std::hex >> battery_status >> buff >> fields_[0];


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

  return true;
}

// ACPI provides a lot of info,
// but munging it into something usefull is ugly
// esp. as you can have more than one battery ...

bool BtryMeter::getacpiinfo( void ){

  DIR *dir = opendir(ACPIBATTERYDIR);
  if (dir==NULL) {
    XOSDEBUG("ACPI: Cannot open directory : %s\n", ACPIBATTERYDIR);
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
  for (struct dirent *dirent; (dirent = readdir(dir)) != NULL; ) {
    if (strncmp(dirent->d_name, ".", 1) == 0
    	|| strncmp(dirent->d_name, "..", 2) == 0)
      continue;

    std::string abs_battery_name = abs_battery_dir + "/" + dirent->d_name;

    XOSDEBUG("ACPI Batt: %s\n", dirent->d_name);

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

  closedir(dir);

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

  return true;

}

// present yes/no can change anytime !
// by adding/removing a battery
bool BtryMeter::battery_present(const std::string& filename)
{
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
  XOSDEBUG("batt %s not present\n",filename.c_str() );
  return false;
}

bool BtryMeter::parse_battery(const std::string& dirname)
{
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
	//XOSDEBUG("alarm: a=\"%s\" v=\"%s\"\n",argname.c_str(),argval.c_str() );

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
