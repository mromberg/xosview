//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "fsmeter.h"



static const char * const MOUNT_FNAME = "/etc/mnttab";



std::vector<std::string> FSMeterFactory::getAuto(void) {
    //  Create a list of entries in mounts where
    //  the type is zfs or tmpfs

    std::vector<std::string> rval;

    std::ifstream ifs(MOUNT_FNAME);
    if (!ifs) {
        logProblem << "Could not open: " << MOUNT_FNAME << std::endl;
        return rval;
    }

    while (!ifs.eof()) {
        std::string dev, path, type, line;
        ifs >> dev >> path >> type;
        logDebug << dev << " -> " << path << " : " << type << std::endl;
        std::getline(ifs, line);
        if (ifs && (type == "zfs" || type == "tmpfs"))
                rval.push_back(path);
    }

    return rval;
}


bool FSMeter::isMount(const std::string &path) {
    std::ifstream ifs(MOUNT_FNAME);
    if (!ifs) {
        logProblem << "Could not open: " << MOUNT_FNAME << std::endl;
        return false;
    }

    while (!ifs.eof()) {
        std::string dev, mpath, type, line;
        ifs >> dev >> mpath >> type;
        std::getline(ifs, line);
        if (ifs && (mpath == path))
            return true;
    }

    return false;
}
