//
//  Copyright (c) 2014, 2015, 2018 by Tomi Tapper <tomi.o.tapper@jyu.fi>
//
//  File based on bsd/intratemeter.* by
//  Copyright (c) 1999 by Brian Grayson (bgrayson@netbsd.org)
//
//  This file may be distributed under terms of the GPL
//
//

#include "intratemeter.h"



IrqRateMeter::IrqRateMeter(kstat_ctl_t *kc)
    : ComIrqRateMeter(),
      _lastirqcount(0),
      _kc(kc),
      _cpus(KStatList::getList(_kc, KStatList::CPU_SYS)) {
}


float IrqRateMeter::getIrqRate(void) {

    uint64_t irqcount = 0;

    _cpus->update(_kc);
    timerStop();

    for (size_t i = 0 ; i < _cpus->count() ; i++) {
        if (kstat_read(_kc, cpus()[i], nullptr) == -1)
            logFatal << "kstat_read() failed." << std::endl;

        kstat_named_t *k = KStatList::lookup(cpus()[i], "intr");
        if (k == nullptr)
            logFatal << "kstat_data_lookup() failed." << std::endl;

        irqcount += kstat_to_ui64(k);
    }
    if (_lastirqcount == 0)
        _lastirqcount = irqcount;

    float rval = (irqcount - _lastirqcount) / etimeSecs();
    _lastirqcount = irqcount;
    timerStart();

    return rval;
}
