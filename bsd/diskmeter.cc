//
//  Copyright (c) 1995, 1996, 1997-2002, 2015
//  by Brian Grayson (bgrayson@netbsd.org)
//
//  This file was written by Brian Grayson for the NetBSD and xosview
//    projects.
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//

#include "diskmeter.h"
#include "kernel.h"


DiskMeter::DiskMeter( XOSView *parent )
    : ComDiskMeter( parent ),
      prevreads_(0), prevwrites_(0) {

    if (!BSDDiskInit())
        logFatal << "BSDDiskInit() failed." << std::endl;

    BSDGetDiskXFerBytes(prevreads_, prevwrites_);
    IntervalTimerStart();
}


std::pair<double, double> DiskMeter::getRate(void) {
    uint64_t reads = 0, writes = 0;
    IntervalTimerStop();
    double t = IntervalTimeInSecs();
    BSDGetDiskXFerBytes(reads, writes);
    IntervalTimerStart();

#if defined(HAVE_DEVSTAT)
    std::pair<double, double> rval(reads / t, writes / t);
#else
    std::pair<double, double> rval((reads - prevreads_) / t,
      (writes - prevwrites_) / t);
    prevreads_ = reads;
    prevwrites_ = writes;
#endif

    return rval;
}
