//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "fsmeter.h"



std::vector<std::string> FSMeterFactory::getAuto(void) {
    //  Create a list of entries in mounts where the device
    //  and mount point are absolute paths.

    std::vector<std::string> rval;

    std::ifstream ifs(MOUNT_FNAME);
    if (!ifs) {
        logProblem << "Could not open: " << MOUNT_FNAME << std::endl;
        return rval;
    }

    while (!ifs.eof()) {
        std::string dev, path, type, fill, line;
        ifs >> dev >> fill >> path >> fill >> type;
        std::getline(ifs, line);
        if (ifs) {
            if (dev[0] == '/' && path[0] == '/')
                rval.push_back(path);
        }
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
        std::string dev, mpath, type, line, fill;
        ifs >> dev >> fill >> mpath >> fill >> type;
        std::getline(ifs, line);
        if (ifs && (mpath == path))
            return true;
    }

    return false;
}
