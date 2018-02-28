//
//  Copyright (c) 1999, 2015, 2018
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//

#include "pagemeter.h"

#include <unistd.h>
#include <sys/sysinfo.h>



PageMeter::PageMeter(kstat_ctl_t *kc)
    : ComPageMeter(), _psize(sysconf(_SC_PAGESIZE)),
      _pageinfo(2, std::vector<float>(2, 0.0)),
      _pageindex(0),
      _cpustats(KStatList::getList(_kc, KStatList::CPU_STAT)),
      _kc(kc) {

    timerStart();
}


std::pair<float, float> PageMeter::getPageRate(void) {
    _pageinfo[_pageindex][0] = 0; // pgin
    _pageinfo[_pageindex][1] = 0; // pgout

    _cpustats->update(_kc);

    for (size_t i = 0 ; i < _cpustats->count() ; i++) {
        cpu_stat_t cs;
        if (kstat_read(_kc, (*_cpustats)[i], &cs) == -1)
            continue;

        _pageinfo[_pageindex][0] += cs.cpu_vminfo.pgpgin;
        _pageinfo[_pageindex][1] += cs.cpu_vminfo.pgpgout;
    }

    const size_t oldindex = (_pageindex + 1) % 2;
    std::vector<uint64_t> piov(2, 0);
    for (size_t i = 0 ; i < piov.size() ; i++) {
        if (_pageinfo[oldindex][i] == 0)
            _pageinfo[oldindex][i] = _pageinfo[_pageindex][i];

        piov[i] = _pageinfo[_pageindex][i] - _pageinfo[oldindex][i];
    }

    _pageindex = (_pageindex + 1) % 2;

    timerStop();
    const double t = etimeSecs();
    timerStart();

    std::pair<float, float> rval((piov[0] * _psize) / t,
      (piov[1] * _psize) / t);

    return rval;
}
