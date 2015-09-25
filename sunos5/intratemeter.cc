//
//  Copyright (c) 2014, 2015 by Tomi Tapper <tomi.o.tapper@jyu.fi>
//
//  File based on bsd/intratemeter.* by
//  Copyright (c) 1999 by Brian Grayson (bgrayson@netbsd.org)
//
//  This file may be distributed under terms of the GPL
//
//

#include "intratemeter.h"



IrqRateMeter::IrqRateMeter(XOSView *parent, kstat_ctl_t *kc)
    : ComIrqRateMeter(parent),
      _lastirqcount(0),
      _kc(kc),
      _cpus(KStatList::getList(_kc, KStatList::CPU_SYS)) {
}


float IrqRateMeter::getIrqRate(void) {
    kstat_named_t *k;
    uint64_t irqcount = 0;

    _cpus->update(_kc);
    IntervalTimerStop();
    for (size_t i = 0; i < _cpus->count(); i++) {
        if (kstat_read(_kc, (*_cpus)[i], NULL) == -1) {
            logFatal << "kstat_read() failed." << std::endl;
        }
        k = (kstat_named_t *)kstat_data_lookup((*_cpus)[i],
          const_cast<char *>("intr"));
        if (k == NULL)
            logFatal << "kstat_data_lookup() failed." << std::endl;

        irqcount += kstat_to_ui64(k);
    }
    if (_lastirqcount == 0)
        _lastirqcount = irqcount;

    float rval = (irqcount - _lastirqcount) / IntervalTimeInSecs();
    _lastirqcount = irqcount;
    IntervalTimerStart();

    return rval;
}
