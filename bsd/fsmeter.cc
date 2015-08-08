//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "fsmeter.h"
#include "fsutil.h"
#include "strutil.h"

#include <cerrno>


#include <sys/types.h>
#include <sys/statvfs.h>

#if defined(XOSVIEW_NETBSD)
static const int XOS_NO_WAIT = ST_NOWAIT;
#elif defined(XOSVIEW_FREEBSD) || defined(XOSVIEW_OPENBSD)
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
static const int XOS_NO_WAIT = MNT_NOWAIT;
#endif



FSMeter::FSMeter(XOSView *parent, const std::string &path)
    : FieldMeterGraph(parent, 2, "FS", "USED/FREE", true, true, true),
      _bgColor(0), _umountColor(0), _path(path){

    legend(_path + ":FREE", ":");
}


FSMeter::~FSMeter(void) {
}


void FSMeter::checkResources(const ResDB &rdb) {

    FieldMeterGraph::checkResources(rdb);

    _bgColor = parent_->g().allocColor(rdb.getResource(
          "filesysBGColor"));
    _umountColor = parent_->g().allocColor(rdb.getResource(
          "filesysNoneColor"));
    setfieldcolor(0, rdb.getResource("filesysFGColor"));
    setfieldcolor(1, _bgColor);

    priority_ = util::stoi(rdb.getResource("filesysPriority"));
    dodecay_ = rdb.isResourceTrue("filesysDecay");
    useGraph_ = rdb.isResourceTrue("filesysGraph");
    setUsedFormat(rdb.getResource("filesysUsedFormat"));
}


void FSMeter::checkevent( void ) {
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

std::vector<std::string> FSMeter::mounts(const ResDB &rdb) {
    std::string mounts = rdb.getResource("filesysMounts");

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

#if defined(XOSVIEW_NETBSD)
    struct statvfs *mntbufp;
#elif defined (XOSVIEW_FREEBSD) || defined(XOSVIEW_OPENBSD)
    struct statfs *mntbufp;
#endif

    int n = getmntinfo(&mntbufp, XOS_NO_WAIT);
    if (n == 0) {
        logProblem << "getmntinfo() failed: " << util::strerror(errno)
                   << std::endl;
        return rval;
    }

    for (int i = 0 ; i < n ; i++) {
        if (mntbufp[i].f_mntfromname[0] == '/'
          && mntbufp[i].f_mntonname[0] == '/')
            rval.push_back(mntbufp[i].f_mntonname);

        logDebug << mntbufp[i].f_mntfromname << " -> "
                 << mntbufp[i].f_mntonname << std::endl;
    }

    return rval;
}


bool FSMeter::isMount(const std::string &path) {

#if defined(XOSVIEW_NETBSD)
    struct statvfs *mntbufp;
#elif defined(XOSVIEW_FREEBSD) || defined(XOSVIEW_OPENBSD)
    struct statfs *mntbufp;
#endif

    int n = getmntinfo(&mntbufp, XOS_NO_WAIT);
    if (n == 0) {
        logProblem << "getmntinfo() failed: " << util::strerror(errno)
                   << std::endl;
        return false;
    }

    for (int i = 0 ; i < n ; i++) {
        if (path == mntbufp[i].f_mntonname)
            return true;
    }

    return false;
}
