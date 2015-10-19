//
//  Copyright (c) 1994, 1995, 2015 by Mike Romberg ( romberg@fsl.noaa.gov )
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

#include "netmeter.h"
#include "kernel.h"




NetMeter::NetMeter( void )
    : ComNetMeter(),
      lastBytesIn_(0), lastBytesOut_(0),
      netIface_("False"), ignored_(false) {

    if (!BSDNetInit())
        logFatal << "The kernel does not seem to have the symbols needed "
                 << "for the NetMeter." << std::endl;
}


void NetMeter::checkResources(const ResDB &rdb) {

    ComNetMeter::checkResources(rdb);

    netIface_ = rdb.getResource("netIface");
    if (netIface_[0] == '-') {
        ignored_ = true;
        netIface_.erase(0, netIface_.find_first_not_of("- "));
    }
}


std::pair<float, float> NetMeter::getRates(void) {
    static bool first = true;
    if (first) {
        first = false;
        BSDGetNetInOut(lastBytesIn_, lastBytesOut_, netIface_,
          ignored_);
        IntervalTimerStart();
        return std::make_pair(0.0, 0.0);
    }

    uint64_t nowBytesIn, nowBytesOut;

    IntervalTimerStop();
    BSDGetNetInOut(nowBytesIn, nowBytesOut, netIface_, ignored_);
    double t = (1.0) / IntervalTimeInSecs();
    IntervalTimerStart();

    std::pair<float, float> rval((nowBytesIn - lastBytesIn_) * t,
      (nowBytesOut - lastBytesOut_) * t);
    lastBytesIn_ = nowBytesIn;
    lastBytesOut_ = nowBytesOut;

    return rval;
}
