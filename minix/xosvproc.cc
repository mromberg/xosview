//
//  Copyright (c) 2015, 2017, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "xosvproc.h"
#include "log.h"
#include "fsutil.h"
#include "strutil.h"

#include <fstream>

#include <minix/procfs.h>



std::ostream &XOSVProc::dump(std::ostream &os) const {
    os << "name=" << name << ",pid=" << pid
       << ",ptype=" << ptype << ",state=" << state
       << ",execycles=" << execycles;

    return os;
}


//------------------------------------------------------------------------
// from 3.4.0/minix/fs/procfs/pid.c
//------------------------------------------------------------------------
//	/* Print all the information. */
// 	buf_printf("%d %c %d %s %c %d %d %u %u "
// 	    "%"PRIu64" %"PRIu64" %"PRIu64" %lu %d %u\n",
// 	    PSINFO_VERSION,			/* information version */
// 	    type,				/* process type */
// 	    mpd.mpd_endpoint,			/* process endpoint */
// 	    mpd.mpd_name,			/* process name */
// 	    state,				/* process state letter */
// 	    mpd.mpd_blocked_on,			/* endpt blocked on, or NONE */
// 	    mpd.mpd_priority,			/* process priority */
// 	    mpd.mpd_user_time,			/* user time */
// 	    mpd.mpd_sys_time,			/* system time */
// 	    mpd.mpd_cycles,			/* execution cycles */
// 	    mpd.mpd_kipc_cycles,		/* kernel IPC cycles */
// 	    mpd.mpd_kcall_cycles,		/* kernel call cycles */
// 	    vui.vui_total,			/* total memory */
// 	    mpd.mpd_nice,			/* nice value */
// 	    uid					/* effective user ID */
// 	);
//--------------------------------------------------------------------------

std::istream &XOSVProc::load(std::istream &is) {
    // The theory is that the first value PSINFO_VERSION will be incremented
    // when the contents of the file change.   This did not seem to happen
    // (yet) for the change between 3.3 and 3.4.  So, this check may be
    // just for theatrics.
    is >> psiVers;
    if (psiVers != PSINFO_VERSION) {
        is.setstate(std::ios::failbit);
        logFatal << "psinfo version: " << psiVers
                 << " does not match: " << PSINFO_VERSION
                 << std::endl;
        return is;
    }

    is >> ptype >> pend >> name >> state >> blockedon >> ppri >> usrtime
       >> systime >> execycles >> kerncycles >> kcallcycles >> totmem
       >> nice >> euid;

    return is;
}


std::vector<XOSVProc> XOSVProc::ptable(void) {

    std::vector<XOSVProc> rval;

    std::vector<std::string> dir(util::fs::listdir("/proc"));
    rval.reserve(dir.size());

    for (size_t i = 0 ; i < dir.size() ; i++) {
        // Skip directory entires that are not numbers.
        if (!std::isdigit(dir[i][0]) && dir[i][0] != '-')
            continue;

        std::string psinfo("/proc/" + dir[i] + "/psinfo");
        std::ifstream ifs(psinfo.c_str());
        if (ifs.good()) {
            XOSVProc p;
            ifs >> p;
            if (ifs.good()) {
                pid_t pid = std::stoi(dir[i]);
                p.pid = pid;
                rval.push_back(p);
            }
        }
    }

    return rval;
}
