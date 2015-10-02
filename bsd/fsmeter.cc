//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "fsmeter.h"

#include <cerrno>

#include <sys/types.h>
#include <sys/statvfs.h>

#if defined(XOSVIEW_NETBSD)
static const int XOS_NO_WAIT = ST_NOWAIT;
#else
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
static const int XOS_NO_WAIT = MNT_NOWAIT;
#endif



bool FSMeter::isMount(const std::string &path) {

#if defined(XOSVIEW_NETBSD)
    struct statvfs *mntbufp;
#else
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


std::vector<std::string> FSMeterFactory::getAuto(void) {
    //  Create a list of entries in mounts where the device
    //  and mount point are absolute paths.

    std::vector<std::string> rval;

#if defined(XOSVIEW_NETBSD)
    struct statvfs *mntbufp;
#else
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
