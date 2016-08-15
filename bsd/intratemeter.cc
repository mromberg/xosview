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


IrqRateMeter::IrqRateMeter( void )
    : ComIrqRateMeter(), _irqcount(0) {

    _istats.scan();
    _irqcount = _istats.maxirq();
    _irqs.resize(_irqcount + 1, 0);
    _lastirqs.resize(_irqcount + 1, 0);

    _istats.counts(_lastirqs);
    IntervalTimerStart();
}


float IrqRateMeter::getIrqRate(void) {
    IntervalTimerStop();
    double t = IntervalTimeInSecs();
    _istats.counts(_irqs);
    IntervalTimerStart();

    long int delta = 0;
    for (size_t i = 0 ; i <= _irqcount ; i++) {
        delta += _irqs[i] - _lastirqs[i];
        _lastirqs[i] = _irqs[i];
        _irqs[i] = 0;
    }

    return (float)delta / t;
}
