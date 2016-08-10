//
//  Copyright (c) 1994, 1995, 2015, 2016 by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  NetBSD port:
//  Copyright (c) 1995, 1996, 1997-2002 by Brian Grayson (bgrayson@netbsd.org)
//
//  This file was originally written by Brian Grayson for the NetBSD and
//    xosview projects.
//
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//

#include "pagemeter.h"
#include "kernel.h"

#include <unistd.h>



PageMeter::PageMeter( void )
    : ComPageMeter(), _psize(sysconf(_SC_PAGESIZE)),
      previnfo_(2, 0) {

    BSDGetPageStats(previnfo_);
    IntervalTimerStart();
}


std::pair<float, float> PageMeter::getPageRate(void) {

    IntervalTimerStop();
    double t = IntervalTimeInSecs();
    std::vector<uint64_t> info;
    BSDGetPageStats(info);
    IntervalTimerStart();

    std::pair<float, float> rval(((info[0] - previnfo_[0]) * _psize) / t ,
      ((info[1] - previnfo_[1]) * _psize) / t);

    previnfo_[0] = info[0];
    previnfo_[1] = info[1];

    return rval;
}
