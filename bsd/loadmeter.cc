//
//  Copyright (c) 1994, 1995, 2015 by Mike Romberg ( romberg@fsl.noaa.gov )
//  Copyright (c) 1995, 1996, 1997-2002 by Brian Grayson (bgrayson@netbsd.org)
//
//  Most of this code was written by Werner Fink <werner@suse.de>.
//  Only small changes were made on my part (M.R.)
//  And the near-trivial port to NetBSD was done by Brian Grayson
//
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//

#include "loadmeter.h"
#include "kernel.h"



LoadMeter::LoadMeter( void )
    : ComLoadMeter() {
}


float LoadMeter::getLoad(void) {
    double oneMinLoad;
    getloadavg(&oneMinLoad, 1);  //  Only get the 1-minute-average sample.

    return oneMinLoad;
}


uint64_t LoadMeter::getCPUSpeed(void) {
    return BSDGetCPUSpeed() * 1000000;
}
