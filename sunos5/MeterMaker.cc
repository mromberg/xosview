//
//  Copyright (c) 1999, 2015, 2016
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




MeterMaker::MeterMaker(void) {
}


std::vector<Meter *> MeterMaker::makeMeters(const ResDB &rdb) {

    kstat_ctl_t *kc = kstat_open();
    if (kc == NULL)
        logFatal << "kstat_open() failed." << std::endl;


    // Add the example meter.  Normally you would use
    // isResourceTrue.  But example resources are not in Xdefalts
    if (rdb.getResourceOrUseDefault("example", "False") == "True")
        _meters.push_back(new ExampleMeter());

    // Standard meters (usually added, but users could turn them off)
    if (rdb.isResourceTrue("load"))
        _meters.push_back(new LoadMeter(kc));

    if (rdb.isResourceTrue("cpu"))
        cpuFactory(rdb, kc);

    if (rdb.isResourceTrue("mem"))
        _meters.push_back(new MemMeter(kc));

    if (rdb.isResourceTrue("disk"))
        _meters.push_back(new DiskMeter(kc));

    if (rdb.isResourceTrue("filesys"))
        util::concat(_meters, FSMeterFactory().make(rdb));

    if (rdb.isResourceTrue("swap"))
        _meters.push_back(new SwapMeter());

    if (rdb.isResourceTrue("page"))
        _meters.push_back(new PageMeter(kc));

    if (rdb.isResourceTrue("net"))
        _meters.push_back(new NetMeter(kc));

    if (rdb.isResourceTrue("irqrate"))
        _meters.push_back(new IrqRateMeter(kc));

    return _meters;
}


void MeterMaker::cpuFactory(const ResDB &rdb, kstat_ctl_t *kc) {
    int cpuCount = sysconf(_SC_NPROCESSORS_ONLN);

    size_t start = 0, end = 0;
    getRange(rdb.getResource("cpuFormat"), cpuCount, start, end);

    KStatList *cpulist = KStatList::getList(kc, KStatList::CPU_STAT);
    for (size_t i = start ; i <= end ; i++)
        if (i == 0)
            _meters.push_back(new CPUMeter(kc, -1));
        else
            _meters.push_back(new CPUMeter(kc,
                (*cpulist)[i - 1]->ks_instance));
}
