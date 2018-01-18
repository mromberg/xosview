//
//  Copyright (c) 1999, 2006, 2015, 2016, 2018
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
      _dir("/sys/block/" + _device + "/md/"),
      _ffsize(0), _ffColor(0), _degradedColor(0) {

    _level = util::strip(util::fs::readAll(_dir + "level"));

    if (!util::fs::readFirst(_dir + "raid_disks", _ffsize))
        logProblem << "reading " << _dir + "raid_disks  failed." << std::endl;

    // legend format: level ndevs/ffsize - sync_action - TODO
    legend(_level + " none ?/" + util::repr(_ffsize) + " - none - TODO", "-");

    _total = 1.0;         // All fields 0.0 - 1.0
    // _fields[0] = 0.0;  // Always 0.0 (used for legend color).
    // _fields[1] = 0.0;  // sync_action progress.
    // _fields[2] = 0.0;  // todo
}


RAIDMeter::~RAIDMeter( void ){
}


void RAIDMeter::checkevent( void ) {

    // Set the bits and count active devices.
    size_t active = setDevBits();

    // Set the fieldmeter based on the current sync_action (if any).
    std::string sync_action = setSyncAction();

    std::string state(util::strip(util::fs::readAll(_dir + "array_state")));

    // Set the color of field 0 (used for legend color) and set the legend.
    setfieldcolor(0, active >= _ffsize ? _ffColor : _degradedColor);
    legend(_level + " " + state + " "
      + util::repr(active) + "/" + util::repr(_ffsize)
      + " - " + sync_action + " - TODO", "-");
}


void RAIDMeter::checkResources(const ResDB &rdb){
    BitFieldMeter::checkResources(rdb);

    // fully functional and degraded colors.
    _ffColor = rdb.getColor("RAIDfullColor");
    _degradedColor = rdb.getColor("RAIDdegradedColor");

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
    setfieldcolor(0, _ffColor);
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


std::vector<std::string> RAIDMeter::scanMDDevs(void) {
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

    std::vector<std::string> devs(scanMDDevs());
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


std::vector<std::string> RAIDMeter::scanDevs(void) {
    std::vector<std::string> devs;

    std::vector<std::string> mddir(util::fs::listdir(_dir));
    for (size_t i = 0 ; i < mddir.size() ; i++)
        if (mddir[i].substr(0, 4) == "dev-")
            devs.push_back(mddir[i]);

    return devs;
}


const std::map<std::string, unsigned char> &RAIDMeter::devState(void) {
    static std::map<std::string, unsigned char> m;
    static bool first = true;

    if (first) {
        first = false;
        m["in_sync"]          = 1;
        m["writemostly"]      = 2;
        m["replacement"]      = 3;
        m["spare"]            = 4;
        m["write_error"]      = 5;
        m["want_replacement"] = 6;
        m["blocked"]          = 7;
        m["faulty"]           = 8;
    }

    return m;
}


size_t RAIDMeter::setDevBits(void) {
    size_t active = 0;

    std::vector<std::string> devs(scanDevs());
    if (devs.size() != numbits())
        setNumBits(devs.size());

    for (size_t i = 0 ; i < devs.size() ; i++) {
        std::string devdir(_dir + devs[i] + "/");

        // if the file slot exists and does not contain "none" it is active.
        if (util::fs::isfile(devdir + "slot")) {
            std::string slot(util::strip(util::fs::readAll(devdir + "slot")));
            if (slot != "none")
                active++;
        }

        // set the bit based on the device state.
        std::string state(util::strip(util::fs::readAll(devdir + "state")));
        std::string fstate(filterState(state));
        logDebug << _device << ", " << devs[i] << ",  state: "
                 << state << " : " << fstate << std::endl;

        _bits[i] = util::get(devState(), fstate);
    }

    return active;
}


std::string RAIDMeter::setSyncAction(void) {

    std::string sync_action("idle");

    _fields[1] = 0.0;
    _fields[2] = 1.0;

    // Not all raid levels have a sync_action.  If missing use idle.
    if (util::fs::readAll(_dir + "sync_action", sync_action)) {
        sync_action = util::strip(sync_action);

        // The sync_completed file is number / number (or none).
        std::string compstr(util::fs::readAll(_dir + "sync_completed"));
        if (util::strip(compstr) != "none") {
            std::istringstream is(compstr);
            float done = 0.0, total = 1.0;
            std::string buff;
            is >> done >> buff >> total;
            if (is.fail() || buff != "/") {
                logProblem << "failed to parse: " << _dir + "sync_completed"
                           << std::endl;
            }
            else {
                _fields[1] = done / total;
                _fields[2] = 1.0 - _fields[1];
            }
        }
    }

    setfieldcolor(1, _actionColors[sync_action]);

    return sync_action;
}


std::string RAIDMeter::filterState(const std::string &state) const {
    // Contents of the device state file is a ',' separated list of states.
    // Try and pick the most relevant one.
    std::vector<std::string> states(util::split(state, ","));
    for (size_t i = 0 ; i < states.size() ; i++)
        states[i] = util::strip(states[i]);

    if (util::find(states, std::string("faulty")))
        return "faulty";

    if (util::find(states, std::string("spare")))
        return "spare";

    if (util::find(states, std::string("in_sync"))) {
        if (util::find(states, std::string("blocked")))
            return "blocked";
        if (util::find(states, std::string("want_replacement")))
            return "want_replacement";
        if (util::find(states, std::string("write_error")))
            return "write_error";
        if (util::find(states, std::string("write_mostly")))
            return "writemostly";
        if (util::find(states, std::string("replacement")))
            return "replacement";

        return "in_sync";
    }

    // If we get here then pick something.
    if (!states.empty())
        return states[0];

    return "none";
}
