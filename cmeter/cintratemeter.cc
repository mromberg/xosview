//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "cintratemeter.h"



ComIrqRateMeter::ComIrqRateMeter( XOSView *parent )
    : FieldMeterGraph( parent, 2, "IRQR", "IRQs per sec/IDLE", 1, 1, 0 ),
      _peak(1000) {
}


ComIrqRateMeter::~ComIrqRateMeter( void ) {
}


void ComIrqRateMeter::checkResources(const ResDB &rdb) {
    FieldMeterGraph::checkResources(rdb);
    setfieldcolor(0, rdb.getColor("irqrateUsedColor"));
    setfieldcolor(1, rdb.getColor("irqrateIdleColor"));

    _peak = util::stof(rdb.getResource("irqratePeak"));
    total_ = _peak;
}


void ComIrqRateMeter::checkevent( void ){

    float rate = getIrqRate();

    setUsed(rate, 1.0);

    if (rate > _peak)
        rate = _peak;

    fields_[0] = rate;
    fields_[1] = total_ - rate;
}
