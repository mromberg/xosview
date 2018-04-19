//
//  Copyright (c) 2015, 2018
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//

#include "loadmeter.h"

#include <sys/loadavg.h>



LoadMeter::LoadMeter(kstat_ctl_t *_kc)
    : ComLoadMeter(),
      _cpulist(KStatList::getList(_kc, KStatList::CPU_INFO)),
      _kc(_kc), _ksp(nullptr) {
}


float LoadMeter::getLoad(void) {
    // getloadavg() (Solaris 7 and newer).
    double val;
    getloadavg(&val, 1);
    return val;
}


uint64_t LoadMeter::getCPUSpeed(void) {
    _cpulist->update(_kc);

    unsigned int total_mhz = 0;
    size_t count = 0;
    for (size_t i = 0 ; i < _cpulist->count() ; i++) {
        kstat_t *cpu = cpulist()[i];
        if (kstat_read(_kc, cpu, nullptr) == -1)
            logFatal << "kstat_read() failed." << std::endl;

        // Try current_clock_Hz first (needs frequency scaling support),
        // then clock_MHz.
        kstat_named_t *k = nullptr;
        if ((k = KStatList::lookup(cpu, "current_clock_Hz")) == nullptr) {
            if ((k = KStatList::lookup(cpu, "clock_MHz")) == nullptr)
                logFatal << "CPU speed is not available." << std::endl;

            logDebug << "Speed of cpu " << i << " is "
                     << kstat_to_ui64(k) << std::endl;

            total_mhz += kstat_to_ui64(k);
        }
        else {
            logDebug << "Speed of cpu " << i << " is "
                     << kstat_to_ui64(k) << std::endl;

            total_mhz += kstat_to_ui64(k) / 1000000;
        }
        count++;
    }

    const uint64_t speed = count ? total_mhz / count : 0;

    return speed * 1000000;
}
