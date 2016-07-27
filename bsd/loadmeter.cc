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

#include <sys/param.h>
#include <sys/sysctl.h>


LoadMeter::LoadMeter( void )
    : ComLoadMeter() {
}


float LoadMeter::getLoad(void) {
    double oneMinLoad;
    getloadavg(&oneMinLoad, 1);  //  Only get the 1-minute-average sample.

    return oneMinLoad;
}


#if defined(XOSVIEW_NETBSD) || defined(XOSVIEW_DFBSD)

static uint64_t GetCPUSpeed(const std::string &sysname) {
    uint64_t speed = 0;
    size_t size = sizeof(speed);

    if (sysctlbyname(sysname.c_str(), &speed, &size, NULL, 0) < 0)
        logFatal << "sysctl(" << sysname << ") failed" << std::endl;

    return speed / 1000000;
}

#endif

#if defined(XOSVIEW_NETBSD)

uint64_t LoadMeter::getCPUSpeed(void) {
    return GetCPUSpeed("machdep.tsc_freq");
}

#elif defined(XOSVIEW_DFBSD)

uint64_t LoadMeter::getCPUSpeed(void) {
    return GetCPUSpeed("hw.tsc_frequency");
}

#elif defined(XOSVIEW_OPENBSD)

uint64_t LoadMeter::getCPUSpeed(void) {

    const int mib[] = { CTL_HW, HW_CPUSPEED };
    int speed = 0;
    size_t size = sizeof(speed);

    if (sysctl(mib, sizeof(mib)/sizeof(int), &speed, &size, NULL, 0) < 0)
        logFatal << "syscl hw.cpuspeed failed" << std::endl;

    return speed * 1000000;
}

#elif defined(XOSVIEW_FREEBSD)

uint64_t LoadMeter::getCPUSpeed(void) {
    static int cpus = CPUMeter::countCPUs();
    int cpu_speed = 0;
    int speed = 0, avail_cpus = 0;
    size_t size = sizeof(speed);

    for (int i = 0; i < cpus; i++) {
        std::string name("dev.cpu." + util::repr(i) + ".freq");
        if ( sysctlbyname(name.c_str(), &speed, &size, NULL, 0) == 0 ) {
            // count only cpus with individual freq available
            cpu_speed += speed;
            avail_cpus++;
        }
    }
    if (avail_cpus > 1)
        cpu_speed /= avail_cpus;

    return cpu_speed;
}

#endif
