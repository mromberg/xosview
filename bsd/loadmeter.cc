//
//  Copyright (c) 1994, 1995, 2015, 2016 by Mike Romberg ( romberg@fsl.noaa.gov )
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


LoadMeter::LoadMeter( void )
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
#endif


#if defined(XOSVIEW_NETBSD) || defined(XOSVIEW_FREEBSD)

uint64_t LoadMeter::getCPUSpeed(void) {
    return GetCPUSpeed("machdep.tsc_freq");
}

#elif defined(XOSVIEW_DFBSD)

uint64_t LoadMeter::getCPUSpeed(void) {
    return GetCPUSpeed("hw.tsc_frequency");
}

#elif defined(XOSVIEW_OPENBSD)

uint64_t LoadMeter::getCPUSpeed(void) {

    static SysCtl speed_sc;
    if (speed_sc.mib().empty()) {
        speed_sc.mib().push_back(CTL_HW);
        speed_sc.mib().push_back(HW_CPUSPEED);
    }

    int speed = 0;
    if (!speed_sc.get(speed))
        logFatal << "sysctl(hw.cpuspeed) failed." << std::endl;

    return speed * 1000000;
}

#endif
