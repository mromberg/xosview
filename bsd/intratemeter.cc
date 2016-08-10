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
#include "kernel.h"


IrqRateMeter::IrqRateMeter( void )
    : ComIrqRateMeter(), irqcount_(0) {

    irqcount_ = BSDNumInts();
    irqs_.resize(irqcount_ + 1, 0);
    lastirqs_.resize(irqcount_ + 1, 0);

    BSDGetIntrCount(lastirqs_);
    IntervalTimerStart();
}


float IrqRateMeter::getIrqRate(void) {
    IntervalTimerStop();
    double t = IntervalTimeInSecs();
    BSDGetIntrCount(irqs_);
    IntervalTimerStart();

    long int delta = 0;
    for (size_t i = 0; i <= irqcount_; i++) {
        delta += irqs_[i] - lastirqs_[i];
        lastirqs_[i] = irqs_[i];
        irqs_[i] = 0;
    }

    return (float)delta / t;
}
