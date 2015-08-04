//
//  Copyright (c) 1999, 2015
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
#include "intratemeter.h"
#include "fsmeter.h"
#include "example.h"  // The example meter

#include "kstats.h"



MeterMaker::MeterMaker(XOSView *xos) : _xos(xos) {
}


void MeterMaker::makeMeters(void) {

    kstat_ctl_t *kc = kstat_open();
    if (kc == NULL)
        return;

    // Add the example meter.  Normally you would use
    // isResourceTrue.  But example resources are not in Xdefalts
    if (_xos->getResourceOrUseDefault("example", "False") == "True")
        push(new ExampleMeter(_xos));

    // Standard meters (usually added, but users could turn them off)
    if (_xos->isResourceTrue("load"))
        push(new LoadMeter(_xos, kc));

    if (_xos->isResourceTrue("cpu"))
        cpuFactory(kc);

    if (_xos->isResourceTrue("mem"))
        push(new MemMeter(_xos, kc));

    if (_xos->isResourceTrue("disk"))
        push(new DiskMeter(_xos, kc, util::stof(_xos->getResource(
                  "diskBandwidth"))));

    if (_xos->isResourceTrue("filesys")) {
        std::vector<std::string> fs = FSMeter::mounts(_xos);
        for (size_t i = 0 ; i < fs.size() ; i++)
            push(new FSMeter(_xos, fs[i]));
    }

    if (_xos->isResourceTrue("swap"))
        push(new SwapMeter(_xos));

    if (_xos->isResourceTrue("page"))
        push(new PageMeter(_xos, kc, util::stof(_xos->getResource(
                  "pageBandwidth"))));

    if (_xos->isResourceTrue("net"))
        push(new NetMeter(_xos, kc, util::stof(_xos->getResource(
                  "netBandwidth"))));

    if (_xos->isResourceTrue("irqrate"))
        push(new IrqRateMeter(_xos, kc));
}


void MeterMaker::cpuFactory(kstat_ctl_t *kc) {
    bool single, both, all;
    int cpuCount = sysconf(_SC_NPROCESSORS_ONLN);

    single = _xos->getResource("cpuFormat") == "single";
    both = _xos->getResource("cpuFormat") == "both";
    all = _xos->getResource("cpuFormat") == "all";

    if (_xos->getResource("cpuFormat") == "auto") {
        if (cpuCount == 1 || cpuCount > 4)
            single = true;
        else
            all = true;
    }

    if (single || both)
        push(new CPUMeter(_xos, kc, -1));

    if (all || both) {
        KStatList *cpulist = KStatList::getList(kc, KStatList::CPU_STAT);
        for (unsigned int i = 0; i < cpulist->count(); i++)
            push(new CPUMeter(_xos, kc, (*cpulist)[i]->ks_instance));
    }
}
