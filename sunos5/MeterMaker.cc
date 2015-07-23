//
//  Copyright (c) 2015
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//
#include "MeterMaker.h"

#include "cpumeter.h"
#include "memmeter.h"
#include "swapmeter.h"
#include "loadmeter.h"
#include "pagemeter.h"
#include "diskmeter.h"
#include "netmeter.h"
#include "example.h"  // The example meter


#include <kstat.h>


MeterMaker::MeterMaker(XOSView *xos) : _xos(xos) {
}

void MeterMaker::makeMeters(void) {

    // Add the example meter.  Normally you would use
    // isResourceTrue.  But example resources are not in Xdefalts
    if (_xos->getResourceOrUseDefault("example", "False") == "True")
        push(new ExampleMeter(_xos));

    kstat_ctl_t *kc = kstat_open();
    if (kc == NULL)
        return;

    if (_xos->isResourceTrue("load"))
        push(new LoadMeter(_xos, kc));

    // Standard meters (usually added, but users could turn them off)
    if (_xos->isResourceTrue("cpu")) {
        int cpuCount = CPUMeter::countCPUs(kc);
        int start = (cpuCount == 0) ? 0 : 1;
        for (int i = start ; i <= cpuCount ; i++)
            push(new CPUMeter(_xos, kc, i));
    }

    if (_xos->isResourceTrue("mem"))
        push(new MemMeter(_xos, kc));

    if (_xos->isResourceTrue("disk"))
        push(new DiskMeter(_xos, kc, util::stof(_xos->getResource(
                  "diskBandwidth"))));

    if (_xos->isResourceTrue("swap"))
        push(new SwapMeter(_xos));

    if (_xos->isResourceTrue("page"))
        push(new PageMeter(_xos, kc,
            util::stof(_xos->getResource("pageBandwidth"))));

    if (_xos->isResourceTrue("net"))
        push(new NetMeter(_xos, kc, util::stof(_xos->getResource(
                  "netBandwidth"))));
}
