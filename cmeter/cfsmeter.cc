//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "cfsmeter.h"
#include "fsutil.h"



ComFSMeter::ComFSMeter(const std::string &path)
    : FieldMeterGraph(2, "FS", "USED/FREE"),
      _bgColor(0), _umountColor(0), _path(path) {

    legend(_path + ":FREE", ":");
}


void ComFSMeter::checkResources(const ResDB &rdb) {

    FieldMeterGraph::checkResources(rdb);

    _bgColor = rdb.getColor("filesysBGColor");
    _umountColor = rdb.getColor("filesysNoneColor");
    setfieldcolor(0, rdb.getColor("filesysFGColor"));
    setfieldcolor(1, _bgColor);
}


void ComFSMeter::checkevent(void) {
    _total = 1.0;

    if (isMount(_path)) {
        setBGColor(_bgColor);

        // (free, total) (for root)
        const auto fsSize = util::fs::getSpace(_path, true);

        logDebug << _path << ":\t" << fsSize << std::endl;

        _fields[0] = static_cast<float>(fsSize.second - fsSize.first)
            / static_cast<float>(fsSize.second);
        _fields[1] = static_cast<float>(fsSize.first)
            / static_cast<float>(fsSize.second);
        setUsed(fsSize.second - fsSize.first, fsSize.second);
    }
    else {
        _fields[0] = 0;
        _fields[1] = 1.0;
        setUsed(0, 1);
        setBGColor(_umountColor);
    }
}


void ComFSMeter::setBGColor(unsigned long c) {
    if (fieldcolor(1) != c) {
        _fields[0] = 1.0;
        _fields[1] = 0;
        setfieldcolor(1, c);
        _fields[0] = 0;
        _fields[1] = 1.0;
    }
}


bool ComFSMeter::isMount(const std::string &path) {
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
