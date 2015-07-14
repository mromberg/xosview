//
//  Copyright (c) 1994, 1995, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "MeterMaker.h"
#include "xosview.h"

#include "cpumeter.h"
#include "memmeter.h"
#include "swapmeter.h"
#include "loadmeter.h"
#include "pagemeter.h"

#include <stdlib.h>


MeterMaker::MeterMaker(XOSView *xos){
    _xos = xos;
}

void MeterMaker::makeMeters(void){
    if (_xos->isResourceTrue("load"))
        push(new LoadMeter(_xos));
    if (_xos->isResourceTrue("cpu"))
        push(new CPUMeter(_xos));
    if (_xos->isResourceTrue("mem"))
        push(new MemMeter(_xos));
    if (_xos->isResourceTrue("swap"))
        push(new SwapMeter(_xos));

    if (_xos->isResourceTrue("page"))
        push(new PageMeter(_xos, util::stof(_xos->getResource(
                  "pageBandwidth"))));
}
