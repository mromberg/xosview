//
//  Copyright (c) 1999, 2006, 2015, 2016
//  by Thomas Waldmann ( ThomasWaldmann@gmx.de )
//  based on work of Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "raidmeter.h"
#include "fsutil.h"


RAIDMeter::RAIDMeter( int raiddev)
    : BitFieldMeter( 1, 3), _raiddev(raiddev) {

    scan();
    title("md127");
    legend("raid1 clean 2/2 - idle - TODO", "-");
    setNumBits(2);
    total_ = 1.0;
    fields_[0] = 0.0;  // Always 0.0 (used for legend color).
    fields_[1] = 1.0;
    fields_[2] = 0.0;

    _bits[0] = 1;
    _bits[1] = 1;
}


RAIDMeter::~RAIDMeter( void ){
}


void RAIDMeter::checkevent( void ){
}


void RAIDMeter::checkResources(const ResDB &rdb){
    BitFieldMeter::checkResources(rdb);

    // Colors for the fieldmeter (action progress).
    _actionColors["idle"] = rdb.getColor("RAIDidleColor");
    _actionColors["check"] = rdb.getColor("RAIDcheckColor");
    _actionColors["resync"] = rdb.getColor("RAIDresyncColor");
    _actionColors["recover"] = rdb.getColor("RAIDrecoverColor");
    _actionColors["repair"] = rdb.getColor("RAIDrepairColor");

    // Colors of the bit meter (drive status).
    _driveColors["in_sync"] = rdb.getColor("RAIDin_syncColor");
    _driveColors["writemostly"] = rdb.getColor("RAIDwritemostlyColor");
    _driveColors["replacement"] = rdb.getColor("RAIDreplacementColor");
    _driveColors["spare"] = rdb.getColor("RAIDspareColor");
    _driveColors["write_error"] = rdb.getColor("RAIDwrite_errorColor");
    _driveColors["want_replace"] = rdb.getColor("RAIDwant_replaceColor");
    _driveColors["blocked"] = rdb.getColor("RAIDblockedColor");
    _driveColors["faulty"] = rdb.getColor("RAIDfaultyColor");

    // Set field colors.
    setfieldcolor(0, _actionColors["check"]);
    setfieldcolor(1, _actionColors["idle"]);
    setfieldcolor(2, _actionColors["idle"]);

    // Set bit colors.
    _dbits.color(0, _actionColors["idle"]);
    _dbits.color(1, _driveColors["in_sync"]);
    _dbits.color(2, _driveColors["writemostly"]);
    _dbits.color(3, _driveColors["replacement"]);
    _dbits.color(4, _driveColors["spare"]);
    _dbits.color(5, _driveColors["write_error"]);
    _dbits.color(6, _driveColors["want_replace"]);
    _dbits.color(7, _driveColors["blocked"]);
    _dbits.color(8, _driveColors["faulty"]);
}


void RAIDMeter::scan(void) {
    std::vector<std::string> mddevs;
    std::vector<std::string> bdevs(util::fs::listdir("/sys/block"));
    for (size_t i = 0 ; i < bdevs.size() ; i++) {
        if (bdevs[i].substr(0, 2) == "md"
          && util::fs::isdir("/sys/block/" + bdevs[i] + "/md"))
            mddevs.push_back(bdevs[i]);
    }

    for (size_t i = 0 ; i < mddevs.size() ; i++) {
        std::string bdir("/sys/block/" + mddevs[i] + "/md/");

        //-------------------------------------------------
        // level: raid0, raid1, raid5, linear, multipath, faulty
        //        or a number 0, 5, etc.
        //-------------------------------------------------
        std::string level(util::strip(util::fs::readAll(bdir + "level")));

        //-------------------------------------------------
        // state: clear, inactive, suspended, readonly,
        //        read-auto, clean, active, write-pending,
        //        active-idle
        //-------------------------------------------------
        std::string state(util::strip(util::fs::readAll(bdir + "array_state")));

        //-------------------------------------------------
        // sync_action: resync, recover, idle, check,
        //              repair,
        //
        // (only supported on raid levels with redundancy)
        //-------------------------------------------------
        std::string sync_action(util::strip(util::fs::readAll(bdir
              + "sync_action")));

        //-------------------------------------------------
        // sync_completed: number / total
        //                 OR none (none undocumented)
        //-------------------------------------------------
        std::string sync_completed(util::strip(util::fs::readAll(bdir
              + "sync_completed")));

        //-------------------------------------------------
        // raid_disks: number in a fully functional array.
        //-------------------------------------------------
        std::string raid_disks(util::strip(util::fs::readAll(bdir
              + "raid_disks")));

        std::vector<std::string> mddir(util::fs::listdir(bdir));
        std::map<std::string, std::string> devs;

        for (size_t i = 0 ; i < mddir.size() ; i++) {
            if (mddir[i].substr(0, 4) == "dev-") {
                //------------------------------------------------------
                // state: faulty, in_sync, writemostly, blocked, spare,
                //        write_error, want_replacement, replacement
                //------------------------------------------------------
                std::string dev_state(util::strip(util::fs::readAll(bdir
                      + mddir[i] + "/state")));
                devs[mddir[i]] = dev_state;
            }
        }

        std::cout << "--- " << mddevs[i] << " ---\n"
                  << "dir: " << bdir << "\n"
                  << "level: " << level << "\n"
                  << "state: " << state << "\n"
                  << "sync_action: " << sync_action << "\n"
                  << "sync_completed: " << sync_completed << "\n"
                  << "raid_disks: " << raid_disks << "\n"
                  << "devs: " << devs
                  << std::endl;
    }
}
