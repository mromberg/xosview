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

#include <unistd.h>




MeterMaker::MeterMaker(XOSView *xos) : _xos(xos) {
}


std::vector<Meter *> MeterMaker::makeMeters(const ResDB &rdb) {

    kstat_ctl_t *kc = kstat_open();
    if (kc == NULL)
        logFatal << "kstat_open() failed." << std::endl;


    // Add the example meter.  Normally you would use
    // isResourceTrue.  But example resources are not in Xdefalts
    if (rdb.getResourceOrUseDefault("example", "False") == "True")
        _meters.push_back(new ExampleMeter(_xos));

    // Standard meters (usually added, but users could turn them off)
    if (rdb.isResourceTrue("load"))
        _meters.push_back(new LoadMeter(_xos, kc));

    if (rdb.isResourceTrue("cpu"))
        cpuFactory(rdb, kc);

    if (rdb.isResourceTrue("mem"))
        _meters.push_back(new MemMeter(_xos, kc));

    if (rdb.isResourceTrue("disk"))
        _meters.push_back(new DiskMeter(_xos, kc));

    if (rdb.isResourceTrue("filesys")) {
        std::vector<std::string> fs = FSMeter::mounts(rdb);
        for (size_t i = 0 ; i < fs.size() ; i++)
            _meters.push_back(new FSMeter(_xos, fs[i]));
    }

    if (rdb.isResourceTrue("swap"))
        _meters.push_back(new SwapMeter(_xos));

    if (rdb.isResourceTrue("page"))
        _meters.push_back(new PageMeter(_xos, kc));

    if (rdb.isResourceTrue("net"))
        _meters.push_back(new NetMeter(_xos, kc));

    if (rdb.isResourceTrue("irqrate"))
        _meters.push_back(new IrqRateMeter(_xos, kc));

    return _meters;
}


void MeterMaker::cpuFactory(const ResDB &rdb, kstat_ctl_t *kc) {
    bool single, both, all;
    int cpuCount = sysconf(_SC_NPROCESSORS_ONLN);

    single = rdb.getResource("cpuFormat") == "single";
    both = rdb.getResource("cpuFormat") == "both";
    all = rdb.getResource("cpuFormat") == "all";

    if (rdb.getResource("cpuFormat") == "auto") {
        if (cpuCount == 1 || cpuCount > 4)
            single = true;
        else
            all = true;
    }

    if (single || both)
        _meters.push_back(new CPUMeter(_xos, kc, -1));

    if (all || both) {
        KStatList *cpulist = KStatList::getList(kc, KStatList::CPU_STAT);
        for (unsigned int i = 0; i < cpulist->count(); i++)
            _meters.push_back(new CPUMeter(_xos, kc,
                (*cpulist)[i]->ks_instance));
    }
}
