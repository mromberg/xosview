//
//  Copyright (c) 1999, 2015, 2018
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//

#include "pagemeter.h"

#include <unistd.h>
#include <sys/sysinfo.h>



PageMeter::PageMeter(kstat_ctl_t *_kc)
    : ComPageMeter(), _psize(sysconf(_SC_PAGESIZE)),
      pageinfo_(2, std::vector<float>(2, 0.0)),
      pageindex_(0),
      cpustats(KStatList::getList(_kc, KStatList::CPU_STAT)),
      kc(_kc) {

    timerStart();
}


std::pair<float, float> PageMeter::getPageRate(void) {
    cpu_stat_t cs;
    pageinfo_[pageindex_][0] = 0; // pgin
    pageinfo_[pageindex_][1] = 0; // pgout
    cpustats->update(kc);

    for (unsigned int i = 0; i < cpustats->count(); i++) {
        if (kstat_read(kc, (*cpustats)[i], &cs) == -1)
            continue;
        pageinfo_[pageindex_][0] += cs.cpu_vminfo.pgpgin;
        pageinfo_[pageindex_][1] += cs.cpu_vminfo.pgpgout;
    }

    int oldindex = (pageindex_ + 1) % 2;
    std::vector<uint64_t> piov(2, 0);
    for (size_t i = 0; i < piov.size(); i++) {
        if (pageinfo_[oldindex][i] == 0)
            pageinfo_[oldindex][i] = pageinfo_[pageindex_][i];

        piov[i] = pageinfo_[pageindex_][i] - pageinfo_[oldindex][i];
    }

    pageindex_ = (pageindex_ + 1) % 2;

    timerStop();
    double t = etimeSecs();
    timerStart();

    std::pair<float, float> rval((piov[0] * _psize) / t,
      (piov[1] * _psize) / t);

    return rval;
}
