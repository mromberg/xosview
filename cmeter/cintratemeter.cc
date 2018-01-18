//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "cintratemeter.h"



ComIrqRateMeter::ComIrqRateMeter( void )
    : FieldMeterGraph( 2, "IRQR", "IRQs per sec/IDLE" ),
      _peak(1000) {
}


ComIrqRateMeter::~ComIrqRateMeter( void ) {
}


void ComIrqRateMeter::checkResources(const ResDB &rdb) {
    FieldMeterGraph::checkResources(rdb);
    setfieldcolor(0, rdb.getColor("irqrateUsedColor"));
    setfieldcolor(1, rdb.getColor("irqrateIdleColor"));

    _peak = util::stof(rdb.getResource("irqratePeak"));
    _total = _peak;
}


void ComIrqRateMeter::checkevent( void ){

    float rate = getIrqRate();

    setUsed(rate, 1.0);

    if (rate > _peak)
        rate = _peak;

    _fields[0] = rate;
    _fields[1] = _total - rate;
}
