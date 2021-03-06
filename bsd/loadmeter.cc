//
//  Copyright (c) 1994, 1995, 2015, 2016, 2018
//  by Mike Romberg ( romberg@fsl.noaa.gov )
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
#include "cpumeter.h"
#include "sctl.h"

#if defined(XOSVIEW_OPENBSD)
#include <sys/param.h>
#endif


LoadMeter::LoadMeter(void)
    : ComLoadMeter() {
}


float LoadMeter::getLoad(void) {
    double oneMinLoad;
    getloadavg(&oneMinLoad, 1);  //  Only get the 1-minute-average sample.

    return oneMinLoad;
}


#if !defined(XOSVIEW_OPENBSD)
static uint64_t GetCPUSpeed(const std::string &sysname) {

    static SysCtl speed_sc(sysname); // caches the mib.

    uint64_t speed = 0;
    if (!speed_sc.get(speed))
        logFatal << "sysctl(" << speed_sc.id() << ") failed." << std::endl;

    return speed;
}
#else // XOSVIEW_OPENBSD
static uint64_t GetCPUSpeed(void) {

    static SysCtl speed_sc = { CTL_HW, HW_CPUSPEED };

    int speed = 0;
    if (!speed_sc.get(speed))
        logFatal << "sysctl(hw.cpuspeed) failed." << std::endl;

    return static_cast<uint64_t>(speed) * 1000000;
}
#endif

uint64_t LoadMeter::getCPUSpeed(void) {
#if defined(XOSVIEW_NETBSD) || defined(XOSVIEW_FREEBSD)
    return GetCPUSpeed("machdep.tsc_freq");
#elif defined(XOSVIEW_DFBSD)
    return GetCPUSpeed("hw.tsc_frequency");
#elif defined(XOSVIEW_OPENBSD)
    return GetCPUSpeed();
#endif
}
