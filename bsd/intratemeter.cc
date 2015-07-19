//
//  Copyright (c) 1999, 2015 by Brian Grayson (bgrayson@netbsd.org)
//
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//

#include "intratemeter.h"
#include "kernel.h"
#include <stdlib.h>
#include <strings.h>



IrqRateMeter::IrqRateMeter( XOSView *parent )
    : FieldMeterGraph( parent, 2, "IRQs", "IRQs per sec/IDLE", 1, 1, 0 ),
      irqcount_(0) {

    if (!BSDIntrInit()) {
        logProblem << "The kernel does not seem to have the symbols needed "
                   << "for the IrqRateMeter.\n"
                   << "The IrqRateMeter has been disabled." << std::endl;
        disableMeter();
    }

    irqcount_ = BSDNumInts();
    irqs_.resize(irqcount_ + 1, 0);
    lastirqs_.resize(irqcount_ + 1, 0);
}

IrqRateMeter::~IrqRateMeter( void ) {
}

void IrqRateMeter::checkResources( void ) {
    FieldMeterGraph::checkResources();
    setfieldcolor( 0, parent_->getResource("irqrateUsedColor") );
    setfieldcolor( 1, parent_->getResource("irqrateIdleColor") );
    priority_ = util::stoi( parent_->getResource("irqratePriority") );
    dodecay_ = parent_->isResourceTrue("irqrateDecay");
    useGraph_ = parent_->isResourceTrue("irqrateGraph");
    setUsedFormat( parent_->getResource("irqrateUsedFormat") );
    total_ = 2000;

    BSDGetIntrStats(lastirqs_.data(), NULL);
}

void IrqRateMeter::checkevent( void ) {
    getinfo();
    drawfields(parent_->g());
}

void IrqRateMeter::getinfo( void ) {
    int delta = 0;

    IntervalTimerStop();
    BSDGetIntrStats(irqs_.data(), NULL);

    for (uint i = 0; i <= irqcount_; i++) {
        delta += irqs_[i] - lastirqs_[i];
        lastirqs_[i] = irqs_[i];
    }
    for (size_t i = 0 ; i < irqcount_ + 1 ; i++)
        irqs_[i] = 0;

    /*  Scale delta by the priority.  */
    fields_[0] = delta / IntervalTimeInSecs();

    //  Bump total_, if needed.
    if (fields_[0] > total_)
        total_ = fields_[0];

    setUsed(fields_[0], total_);
    IntervalTimerStart();
}
