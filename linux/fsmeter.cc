//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
#include "fsmeter.h"
#include "fsutil.h"

#include <fstream>


static const char * const MOUNT_FNAME = "/proc/mounts";


FSMeter::FSMeter(XOSView *parent, const std::string &path)
    : FieldMeterGraph(parent, 2, "FS", "USED/FREE", true, true, true),
      _path(path){

    legend(_path + ":FREE", ":");
}


FSMeter::~FSMeter(void) {
}


void FSMeter::checkResources( void ) {

    FieldMeterGraph::checkResources();

    // change "template" to real meter name
    setfieldcolor(0, parent_->getResource("filesysForeGround"));
    setfieldcolor(1, parent_->getResource("filesysBackground"));

    priority_ = util::stoi(parent_->getResource("filesysPriority"));
    dodecay_ = parent_->isResourceTrue("filesysDecay");
    useGraph_ = parent_->isResourceTrue("filesysGraph");
    setUsedFormat(parent_->getResource("filesysUsedFormat"));
}


void FSMeter::checkevent( void ) {
    // free, total (for root)
    std::pair<size_t, size_t> fsSize = util::fs::getSpace(_path, true);

    logEvent << _path << ":\t" << fsSize << std::endl;

    total_ = 1.0;
    fields_[0] = (float)(fsSize.second - fsSize.first) / (float)fsSize.second;
    fields_[1] = (float)fsSize.first / (float)fsSize.second;
    setUsed(fsSize.second - fsSize.first, fsSize.second);
    drawfields(parent_->g());
}


std::vector<std::string> FSMeter::mounts(XOSView *xosv) {

    std::vector<std::string> rval;

    std::ifstream ifs(MOUNT_FNAME);
    if (!ifs) {
        logProblem << "Could not open: " << MOUNT_FNAME << std::endl;
        return rval;
    }

    while (!ifs.eof()) {
        std::string dev, path, type, line;
        ifs >> dev >> path >> type;
        std::getline(ifs, line);
        if (ifs) {
            if (dev[0] == '/' && path[0] == '/')
                rval.push_back(path);
        }
    }

    return rval;
}
