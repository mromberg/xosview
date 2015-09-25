//
//  Copyright (c) 2015
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//

#include "loadmeter.h"

#include <unistd.h>

#ifdef HAVE_GETLOADAVG
#include <sys/loadavg.h>
#else

#ifndef FSCALE
#define FSCALE (1<<8)
#endif

#endif



LoadMeter::LoadMeter(XOSView *parent, kstat_ctl_t *_kc)
    : ComLoadMeter(parent),
      cpulist(KStatList::getList(_kc, KStatList::CPU_INFO)),
      kc(_kc), ksp(0) {

#ifndef HAVE_GETLOADAVG
    ksp = kstat_lookup(kc, const_cast<char *>("unix"), 0,
      const_cast<char *>("system_misc"));
    if (ksp == NULL)
        logFatal << "kstat_lookup() failed." << std::endl;
#endif
}


float LoadMeter::getLoad(void) {

#ifndef HAVE_GETLOADAVG
    // This code is mainly for Solaris 6 and earlier, but should work on
    // any version.
    kstat_named_t *k;

    if (kstat_read(kc, ksp, NULL) == -1)
        logFatal << "kstat_read() failed." << std::endl;

    k = (kstat_named_t *)kstat_data_lookup(ksp,
      const_cast<char *>("avenrun_1min"));
    if (k == NULL)
        logFatal << "kstat_data_lookup() failed." << std::endl;

    return kstat_to_double(k) / FSCALE;
#else
    // getloadavg() if found on Solaris 7 and newer.
    double val;
    getloadavg(&val, 1);
    return val;
#endif
}


uint64_t LoadMeter::getCPUSpeed(void) {
    unsigned int total_mhz = 0, i = 0;
    kstat_named_t *k;
    kstat_t *cpu;
    cpulist->update(kc);

    for (i = 0; i < cpulist->count(); i++) {
        cpu = (*cpulist)[i];
        if (kstat_read(kc, cpu, NULL) == -1)
            logFatal << "kstat_read() failed." << std::endl;

        // Try current_clock_Hz first (needs frequency scaling support),
        // then clock_MHz.
        k = (kstat_named_t *)kstat_data_lookup(cpu,
          const_cast<char *>("current_clock_Hz"));
        if (k == NULL) {
            k = (kstat_named_t *)kstat_data_lookup(cpu,
              const_cast<char *>("clock_MHz"));
            if (k == NULL)
                logFatal << "CPU speed is not available." << std::endl;

            logDebug << "Speed of cpu " << i << " is "
                     << kstat_to_ui64(k) << std::endl;

            total_mhz += kstat_to_ui64(k);
        }
        else {
            logDebug << "Speed of cpu " << i << " is "
                     << kstat_to_ui64(k) << std::endl;

            total_mhz += ( kstat_to_ui64(k) / 1000000 );
        }
    }

    uint64_t speed = ( i > 0 ? total_mhz / i : 0 );

    return speed * 1000000;
}
