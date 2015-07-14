//
//  Copyright (c) 1994, 1995, 2002, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//  2007 by Samuel Thibault ( samuel.thibault@ens-lyon.org )
//
//  This file may be distributed under terms of the GPL
//
#include "MeterMaker.h"

#include "memmeter.h"
#include "swapmeter.h"
#include "pagemeter.h"
#include "loadmeter.h"


MeterMaker::MeterMaker(XOSView *xos){
    _xos = xos;
}

void MeterMaker::makeMeters(void){
    if (_xos->isResourceTrue("load"))
        push(new LoadMeter(_xos));

    if (_xos->isResourceTrue("mem"))
        push(new MemMeter(_xos));
    if (_xos->isResourceTrue("swap"))
        push(new SwapMeter(_xos));

    if (_xos->isResourceTrue("page"))
        push(new PageMeter(_xos, util::stof(_xos->getResource(
                  "pageBandwidth"))));
}
