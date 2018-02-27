//
//  Copyright (c) 1999, 2015, 2016, 2018 by Brian Grayson (bgrayson@netbsd.org)
//
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//

#include "intratemeter.h"


IrqRateMeter::IrqRateMeter(void) : ComIrqRateMeter() {
    _istats.scan();
    _lastirqs = _istats.counts();
    timerStart();
}


float IrqRateMeter::getIrqRate(void) {
    timerStop();
    const double t = etimeSecs();
    const std::map<size_t, uint64_t> &cmap = _istats.counts();
    timerStart();

    uint64_t delta = 0;
    for (const auto &c : cmap)
        delta += c.second - util::get(_lastirqs, c.first);
    _lastirqs = cmap;

    return delta / t;
}
