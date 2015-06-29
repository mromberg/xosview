//
//  Copyright (c) 2000, 2006, 2012, 2015
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



static const char PROC_SENSORS_24[] = "/proc/sys/dev/sensors";
static const char PROC_SENSORS_26[] = "/sys/class/hwmon";

LmsTemp::LmsTemp( XOSView *parent, const std::string &filename,
  const std::string &label, const std::string &caption) : FieldMeter( parent,
    3, label, caption, 1, 1, 0 ) {
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
        _highest = util::stoi(caption.substr(p + 1));
    else
        _highest = 100;
}

LmsTemp::~LmsTemp( void ){
}

/* this part is adapted from ProcMeter3.2 */
int  LmsTemp::checksensors(int isproc, const std::string &dir,
  const std::string &filename) {
    bool found = false;

    if (!util::FS::isdir(dir))
        return false;
    else {
        std::vector<std::string> d1 = util::FS::listdir(dir);
        std::string dirname;

        for (size_t d1i = 0 ; !found && d1i < d1.size(); d1i++) {
            const std::string &ent1 = d1[d1i];

            dirname = dir + "/" + ent1;
            if (!isproc)
                dirname += "/device";
            if (util::FS::isdir(dirname)) {
                if (!util::FS::isdir(dirname)) {
                    logProblem << "The directory " << dirname
                               << "exists but cannot be read.\n";
                }
                else {
                    std::vector<std::string> d2 = util::FS::listdir(dirname);
                    for (size_t d2i = 0 ; d2i < d2.size() ; d2i++) {
                        const std::string &e2dn = d2[d2i];
                        std::string f = dirname + "/" + e2dn;
                        if (!util::FS::isfile(f))
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

void LmsTemp::checkResources( void ){
    FieldMeter::checkResources();

    setfieldcolor( 0, parent_->getResource( "lmstempActColor" ) );
    setfieldcolor( 1, parent_->getResource( "lmstempIdleColor") );
    setfieldcolor( 2, parent_->getResource( "lmstempHighColor" ) );

    priority_ = util::stoi (parent_->getResource( "lmstempPriority" ));
    setUsedFormat(parent_->getResource( "lmstempUsedFormat" ) );
}

void LmsTemp::checkevent( void ){
    getlmstemp();

    drawfields(parent_->g());
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
            logFatal << "Can not open file : " << file << std::endl;

        file >> high >> dummy >> fields_[0];
  }
    else {
        std::string f = _filename + "_input";
        std::ifstream file1(f.c_str());
        if ( !file1 )
            logFatal << "Can not open file : " << file1 << std::endl;

        file1 >> fields_[0];

        f = _filename + "_max";
        std::ifstream file2(f.c_str());
        if ( !file2 )
            logFatal << "Can not open file : " << file2 << std::endl;

        file2 >> high;
        high /= 1000; fields_[0] /= 1000;
    }

    total_ = _highest;  // Max temp
    fields_[1] = high - fields_[0];
    if(fields_[1] <= 0) {	// alarm
        fields_[1] = 0;
        setfieldcolor( 0, parent_->getResource( "lmstempHighColor" ) );
    }
    else
        setfieldcolor( 0, parent_->getResource( "lmstempActColor" ) );

    fields_[2] = total_ - fields_[1] - fields_[0];
    if(fields_[2] <= 0) {	// alarm, high was set above 100
        fields_[2] = 0;
        setfieldcolor( 0, parent_->getResource( "lmstempHighColor" ) );
    }
    else
        setfieldcolor( 0, parent_->getResource( "lmstempActColor" ) );
    setUsed (fields_[0], total_);
}
