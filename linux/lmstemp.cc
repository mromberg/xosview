//
//  Copyright (c) 2000, 2006, 2012, 2015, 2016, 2018
//  by Leopold Toetsch <lt@toetsch.at>
//
//  Read temperature entries from /proc/sys/dev/sensors/*/*
//  and display actual and high temperature
//  if actual >= high, actual temp changes color to indicate alarm
//
//  File based on btrymeter.* by
//  Copyright (c) 1997 by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "lmstemp.h"
#include "fsutil.h"

#include <fstream>


static const char * const PROC_SENSORS_24 = "/proc/sys/dev/sensors";
static const char * const PROC_SENSORS_26 = "/sys/class/hwmon";


LmsTemp::LmsTemp( const std::string &filename, const std::string &label,
  const std::string &caption)
    : FieldMeter( 3, label, caption ),
      _highColor(0), _actColor(0) {

    if(!checksensors(1, PROC_SENSORS_24, filename)) {
	if(!checksensors(0, PROC_SENSORS_26, filename)) {
	    logFatal << "Can not find file : " << PROC_SENSORS_24 << "/*/"
                     << filename
                     << " or " << PROC_SENSORS_26 << "/*/device/" << filename
                     << std::endl;
	}
    }
    size_t p = std::string::npos;
    if ((p = caption.rfind('/')) != std::string::npos)
        _highest = std::stoi(caption.substr(p + 1));
    else
        _highest = 100;
}


LmsTemp::~LmsTemp( void ){
}


bool LmsTemp::checksensors(bool isproc, const std::string &dir,
  const std::string &filename) {

    /* this part is adapted from ProcMeter3.2 */

    bool found = false;

    if (!util::fs::isdir(dir))
        return false;
    else {
        std::vector<std::string> d1 = util::fs::listdir(dir);
        std::string dirname;

        for (size_t d1i = 0 ; !found && d1i < d1.size(); d1i++) {
            const std::string &ent1 = d1[d1i];

            dirname = dir + "/" + ent1;
            if (!isproc)
                dirname += "/device";
            if (util::fs::isdir(dirname)) {
                if (!util::fs::isdir(dirname)) {
                    logProblem << "The directory " << dirname
                               << "exists but cannot be read.\n";
                }
                else {
                    for (const auto &e2dn : util::fs::listdir(dirname)) {
                        std::string f = dirname + "/" + e2dn;
                        if (!util::fs::isfile(f))
                            continue;

                        if((isproc && filename == e2dn) ||
                          ((!isproc && filename == e2dn) &&
                            (e2dn.substr(filename.size()) == "_input"))) {
                            if (!isproc)
                                f.resize(f.size()-6);
                            _filename = f;
                            _isproc = isproc;
                            found = true;
                            break;
			}
		    }
		}
	    }
	}
    }
    return found;
}


void LmsTemp::checkResources(const ResDB &rdb) {
    FieldMeter::checkResources(rdb);

    _highColor = rdb.getColor("lmstempHighColor");
    _actColor = rdb.getColor("lmstempActColor");

    setfieldcolor( 0, _actColor );
    setfieldcolor( 1, rdb.getColor( "lmstempIdleColor") );
    setfieldcolor( 2, _highColor );
}


void LmsTemp::checkevent( void ){
    getlmstemp();
}

// Note:
// procentry looks like
// high low actual
//
// if actual >= high alarm is triggered, fan starts and high is set to
//   a higher value by BIOS
//   after fan cooled down the chips, high get's reset
// if  this happens display color of actual is set to HighColor
// this could be very machine depended
//
// a typical entry on my machine (Gericom Overdose 2 XXL, PIII 600) looks like:
//
// $ sensors
// max1617-i2c-0-4e
// Adapter: SMBus PIIX4 adapter at 1400
// Algorithm: Non-I2C SMBus adapter
// temp:       52 C (limit:   55 C, hysteresis:  -55 C)
// remote_temp:
//             56 C (limit:   90 C, hysteresis:  -55 C)
//
// after alarm limits are set to 60 / 127 respectively
// low/hysteresis looks broken ;-)
//

void LmsTemp::getlmstemp( void ){
    // dummy, high changed from integer to double to allow it to display
    // the full value, unfit for an int. (See Debian bug #183695)
    double dummy, high;

    if (_isproc) {
        std::ifstream file( _filename.c_str() );

        if ( !file )
            logFatal << "Can not open file : " << _filename << std::endl;

        file >> high >> dummy >> _fields[0];
  }
    else {
        std::string f = _filename + "_input";
        std::ifstream file1(f.c_str());
        if ( !file1 )
            logFatal << "Can not open file : " << f << std::endl;

        file1 >> _fields[0];

        f = _filename + "_max";
        std::ifstream file2(f.c_str());
        if ( !file2 )
            logFatal << "Can not open file : " << f << std::endl;

        file2 >> high;
        high /= 1000; _fields[0] /= 1000;
    }

    _total = _highest;  // Max temp
    _fields[1] = high - _fields[0];
    if(_fields[1] <= 0) {	// alarm
        _fields[1] = 0;
        setfieldcolor( 0, _highColor );
    }
    else
        setfieldcolor( 0, _actColor );

    _fields[2] = _total - _fields[1] - _fields[0];
    if(_fields[2] <= 0) {	// alarm, high was set above 100
        _fields[2] = 0;
        setfieldcolor( 0, _highColor );
    }
    else
        setfieldcolor( 0, _actColor );
    setUsed (_fields[0], _total);
}
