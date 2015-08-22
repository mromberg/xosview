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


void IrqRateMeter::checkResources(const ResDB &rdb) {
    FieldMeterGraph::checkResources(rdb);
    setfieldcolor( 0, rdb.getColor("irqrateUsedColor") );
    setfieldcolor( 1, rdb.getColor("irqrateIdleColor") );
    priority_ = util::stoi( rdb.getResource("irqratePriority") );
    dodecay_ = rdb.isResourceTrue("irqrateDecay");
    useGraph_ = rdb.isResourceTrue("irqrateGraph");
    setUsedFormat( rdb.getResource("irqrateUsedFormat") );
    total_ = 2000;

    BSDGetIntrCount(lastirqs_);
}


void IrqRateMeter::checkevent( void ) {
    getinfo();
    drawfields(parent_->g());
}


void IrqRateMeter::getinfo( void ) {
    int delta = 0;

    IntervalTimerStop();
    BSDGetIntrCount(irqs_);

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
