//
//  Copyright (c) 1999, 2006, 2015, 2016
//  by Thomas Waldmann ( ThomasWaldmann@gmx.de )
//  based on work of Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "raidmeter.h"
#include "fsutil.h"

#include <algorithm>


RAIDMeter::RAIDMeter(const std::string &device)
    : BitFieldMeter( 1, 3, device), _device(device),
      _dir("/sys/block/" + _device + "/md/"), _ffsize(0) {

    _level = util::strip(util::fs::readAll(_dir + "level"));
    if (!util::fs::readFirst(_dir + "raid_disks", _ffsize))
        logProblem << "reading " << _dir + "raid_disks  failed." << std::endl;

    std::vector<std::string> mddir(util::fs::listdir(_dir));
    for (size_t i = 0 ; i < mddir.size() ; i++)
        if (mddir[i].substr(0, 4) == "dev-")
            _devs.push_back(mddir[i]);

    legend(_level + " none ?/" + util::repr(_ffsize) + " - none - TODO", "-");
    setNumBits(_devs.size());
    total_ = 1.0;         // All fields 0.0 - 1.0
    // fields_[0] = 0.0;  // Always 0.0 (used for legend color).
    // fields_[1] = 0.0;  // sync_action progress.
    // fields_[2] = 0.0;  // todo
}


RAIDMeter::~RAIDMeter( void ){
}


void RAIDMeter::checkevent( void ) {

    size_t active = 0;
    for (size_t i = 0 ; i < _devs.size() ; i++) {
        std::string devdir(_dir + _devs[i] + "/");
        if (util::fs::isfile(devdir + "slot")) {
            std::string slot(util::strip(util::fs::readAll(devdir + "slot")));
            if (slot != "none")
                active++;
        }

        std::string state(util::strip(util::fs::readAll(devdir + "state")));
        unsigned char bstate = 0;
        if (state == "in_sync")
            bstate = 1;
        else if (state == "writemostly")
            bstate = 2;
        else if (state == "replacement")
            bstate = 3;
        else if (state == "spare")
            bstate = 4;
        else if (state == "write_error")
            bstate = 5;
        else if (state == "want_replacement")
            bstate = 6;
        else if (state == "blocked")
            bstate = 7;
        else if (state == "faulty")
            bstate = 8;

        _bits[i] = bstate;
    }

    std::string state(util::strip(util::fs::readAll(_dir + "array_state")));

    legend(_level + " " + state + " " + util::repr(active) + "/" + util::repr(_ffsize)
      + " - none - TODO", "-");
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
    std::vector<std::string> mddevs(scanDevs());

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


std::vector<std::string> RAIDMeter::scanDevs(void) {
    std::vector<std::string> rval;
    std::vector<std::string> bdevs(util::fs::listdir("/sys/block"));

    for (size_t i = 0 ; i < bdevs.size() ; i++) {
        if (bdevs[i].substr(0, 2) == "md"
          && util::fs::isdir("/sys/block/" + bdevs[i] + "/md"))
            rval.push_back(bdevs[i]);
    }

    return rval;
}


std::vector<std::string> RAIDMeter::devices(const ResDB &rdb) {
    std::vector<std::string> rval;

    std::string devices = rdb.getResourceOrUseDefault("RAIDdevices", "auto");
    logDebug << "devices: " << devices << std::endl;

    std::vector<std::string> devs(scanDevs());
    if (devices == "auto") {
        rval = devs;
    }
    else {
        std::vector<std::string> usrDevs = util::split(devices, ",");
        for (size_t i = 0 ; i < usrDevs.size() ; i++) {
            std::string udev(util::strip(usrDevs[i]));
            if (std::find(devs.begin(), devs.end(), udev) != devs.end())
                rval.push_back(udev);
        }
    }

    return rval;
}
