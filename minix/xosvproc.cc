//
//  Copyright (c) 2015
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
#include <minix/u64.h>


std::ostream &XOSVProc::dump(std::ostream &os) const {
    os << "name=" << name << ",pid=" << pid
       <<",ptype=" << ptype << ",state=" << state
       << ",cycles=" << cycles;

    return os;
}


std::istream &XOSVProc::load(std::istream &is) {
    is >> psiVers;
    if (psiVers != PSINFO_VERSION) {
        is.setstate(std::ios::failbit);
        logFatal << "psinfo version: " << psiVers
                 << " does not match: " << PSINFO_VERSION
                 << std::endl;
        return is;
    }

    is >> ptype >> pend >> name >> state >> blockedon
       >> ppri >> usrTime >> sysTime >> cycles_hi >> cycles_lo;
    if (!is)
        return is;
    cycles = make64(cycles_lo, cycles_hi);

    if (ptype != TYPE_TASK) {
        is >> totMem >> comMem >> shMem >> sleepState
           >> ppid >> realuid >> effuid >> procgrp
           >> nice >> f_state >> fp_blk_on >> fp_tty;

        if (!is)
            return is;
    }

    is >> kipc_cycles_hi >> kipc_cycles_lo
       >> kcall_cycles_hi >> kcall_cycles_lo;
    if (!is)
        return is;
    kipc_cycles = make64(kipc_cycles_lo, kipc_cycles_hi);
    kcall_cycles = make64(kcall_cycles_lo, kcall_cycles_hi);

    if (ptype == TYPE_TASK) {
        is >> vui_total;
        if (!is)
            return is;
    }

    return is;
}


std::vector<XOSVProc> XOSVProc::ptable(void) {

    std::vector<XOSVProc> rval;

    std::vector<std::string> dir(util::fs::listdir("/proc"));
    rval.reserve(dir.size());

    for (size_t i = 0 ; i < dir.size() ; i++) {
        if (!std::isdigit(dir[i][0]) && dir[i][0] != '-')
            continue;
        std::string psinfo("/proc/" + dir[i] + "/psinfo");
        std::ifstream ifs(psinfo.c_str());
        if (ifs.good()) {
            XOSVProc p;
            ifs >> p;
            if (ifs.good()) {
                pid_t pid = util::stoi(dir[i]);
                p.pid = pid;
                rval.push_back(p);
            }
        }
    }

    return rval;
}
