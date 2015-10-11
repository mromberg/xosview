//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "cfsmeter.h"
#include "fsutil.h"



ComFSMeter::ComFSMeter(XOSView *parent, const std::string &path)
    : FieldMeterGraph(parent, 2, "FS", "USED/FREE", true, true, true),
      _bgColor(0), _umountColor(0), _path(path){

    legend(_path + ":FREE", ":");
}


ComFSMeter::~ComFSMeter(void) {
}


void ComFSMeter::checkResources(const ResDB &rdb) {

    FieldMeterGraph::checkResources(rdb);

    _bgColor = rdb.getColor("filesysBGColor");
    _umountColor = rdb.getColor("filesysNoneColor");
    setfieldcolor(0, rdb.getColor("filesysFGColor"));
    setfieldcolor(1, _bgColor);
}


void ComFSMeter::checkevent( void ) {
    total_ = 1.0;

    if (isMount(_path)) {
        setBGColor(_bgColor);

        // free, total (for root)
        std::pair<uint64_t, uint64_t> fsSize = util::fs::getSpace(_path, true);

        logDebug << _path << ":\t" << fsSize << std::endl;

        fields_[0] = (float)(fsSize.second - fsSize.first)
            / (float)fsSize.second;
        fields_[1] = (float)fsSize.first / (float)fsSize.second;
        setUsed(fsSize.second - fsSize.first, fsSize.second);
    }
    else {
        fields_[0] = 0;
        fields_[1] = 1.0;
        setUsed(0, 1);
        setBGColor(_umountColor);
    }
}


void ComFSMeter::setBGColor(unsigned long c) {
    if (fieldcolor(1) != c) {
        fields_[0] = 1.0;
        fields_[1] = 0;
        setfieldcolor(1, c);
        fields_[0] = 0;
        fields_[1] = 1.0;
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
