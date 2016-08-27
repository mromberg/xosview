//
//  Copyright (c) 1999, 2015, 2016 by Brian Grayson (bgrayson@netbsd.org)
//
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//

#include "intratemeter.h"


IrqRateMeter::IrqRateMeter( void ) : ComIrqRateMeter() {
    _istats.scan();
    _lastirqs = _istats.counts();
    IntervalTimerStart();
}


float IrqRateMeter::getIrqRate(void) {
    IntervalTimerStop();
    double t = IntervalTimeInSecs();
    const std::map<size_t, uint64_t> &cmap = _istats.counts();
    IntervalTimerStart();

    uint64_t delta = 0;
    std::map<size_t, uint64_t>::const_iterator it;
    for (it = cmap.begin() ; it != cmap.end() ; ++it)
        delta += it->second - util::get(_lastirqs, it->first);
    _lastirqs = cmap;

    return delta / t;
}
