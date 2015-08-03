//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
#include "fsmeter.h"
#include "fsutil.h"
#include "strutil.h"

#include <fstream>


static const char * const MOUNT_FNAME = "/proc/mounts";


FSMeter::FSMeter(XOSView *parent, const std::string &path)
    : FieldMeterGraph(parent, 2, "FS", "USED/FREE", true, true, true),
      _bgColor(0), _umountColor(0), _path(path){

    legend(_path + ":FREE", ":");
}


FSMeter::~FSMeter(void) {
}


void FSMeter::checkResources( void ) {

    FieldMeterGraph::checkResources();


    _bgColor = parent_->g().allocColor(parent_->getResource(
          "filesysBGColor"));
    _umountColor = parent_->g().allocColor(parent_->getResource(
          "filesysNoneColor"));
    setfieldcolor(0, parent_->getResource("filesysFGColor"));
    setfieldcolor(1, _bgColor);

    priority_ = util::stoi(parent_->getResource("filesysPriority"));
    dodecay_ = parent_->isResourceTrue("filesysDecay");
    useGraph_ = parent_->isResourceTrue("filesysGraph");
    setUsedFormat(parent_->getResource("filesysUsedFormat"));
}


void FSMeter::checkevent( void ) {
    total_ = 1.0;

    if (isMount(_path)) {
        setBGColor(_bgColor);

        // free, total (for root)
        std::pair<size_t, size_t> fsSize = util::fs::getSpace(_path, true);

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

    drawfields(parent_->g());
}


void FSMeter::setBGColor(unsigned long c) {
    if (fieldcolor(1) != c) {
        fields_[0] = 1.0;
        fields_[1] = 0;
        setfieldcolor(1, c);
        drawfields(parent_->g());
        fields_[0] = 0;
        fields_[1] = 1.0;
        drawLegend(parent_->g());
    }
}

std::vector<std::string> FSMeter::mounts(XOSView *xosv) {
    std::string mounts = xosv->getResource("filesysMounts");

    logDebug << "MOUNTS: " << mounts << std::endl;

    if (mounts == "auto")
        return getAuto();

    // Then filesysMounts is a list of directories
    std::vector<std::string> rval;
    rval = util::split(mounts, " ");

    logDebug << "MOUNTS: " << rval;

    return rval;
}


std::vector<std::string> FSMeter::getAuto(void) {
    //  Create a list of entries in mounts where the device
    //  and mount point are absolute paths.

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
