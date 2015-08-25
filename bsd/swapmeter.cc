//
//  Copyright (c) 1994, 1995, 2015
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



SwapMeter::SwapMeter( XOSView *parent )
    : FieldMeterGraph( parent, 2, "SWAP", "USED/FREE" ) {

    BSDSwapInit();
}


SwapMeter::~SwapMeter( void ) {
}


void SwapMeter::checkResources(const ResDB &rdb) {

    FieldMeterGraph::checkResources(rdb);

    setfieldcolor( 0, rdb.getColor("swapUsedColor") );
    setfieldcolor( 1, rdb.getColor("swapFreeColor") );
    priority_ = util::stoi( rdb.getResource("swapPriority") );
    dodecay_ = rdb.isResourceTrue("swapDecay");
    useGraph_ = rdb.isResourceTrue("swapGraph");
    setUsedFormat( rdb.getResource("swapUsedFormat") );
}


void SwapMeter::checkevent( void ) {
    getswapinfo();
}


void SwapMeter::getswapinfo( void ) {
    uint64_t total = 0, used = 0;

    BSDGetSwapInfo(total, used);

    total_ = (double)total;
    if ( total_ == 0.0 )
        total_ = 1.0;  /* We don't want any division by zero, now, do we?  :) */
    fields_[0] = (double)used;
    fields_[1] = total_;

    setUsed(fields_[0], total_);
}
