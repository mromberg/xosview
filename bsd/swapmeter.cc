//
//  Copyright (c) 1994, 1995, 2015, 2016
//  by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  NetBSD port:
//  Copyright (c) 1995, 1996, 1997-2002 by Brian Grayson (bgrayson@netbsd.org)
//
//  This file was written by Brian Grayson for the NetBSD and xosview
//    projects.
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//

#include "swapmeter.h"
#include "kernel.h"



SwapMeter::SwapMeter( void )
    : ComSwapMeter() {
}


std::pair<uint64_t, uint64_t> SwapMeter::getswapinfo( void ) {
    uint64_t total = 0, used = 0;

    BSDGetSwapInfo(total, used);

    return std::make_pair(total, total - used);
}
