//
//  Copyright (c) 1999, 2015, 2016, 2018
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



ComMeterMaker::mlist MeterMaker::makeMeters(const ResDB &rdb) {

    mlist meters;

    kstat_ctl_t *kc = kstat_open();
    if (kc == NULL)
        logFatal << "kstat_open() failed." << std::endl;


    // Add the example meter.  Normally you would use
    // isResourceTrue.  But example resources are not in Xdefalts
    if (rdb.getResourceOrUseDefault("example", "False") == "True")
        meters.push_back(std::make_unique<ExampleMeter>());

    // Standard meters (usually added, but users could turn them off)
    if (rdb.isResourceTrue("load"))
        meters.push_back(std::make_unique<LoadMeter>(kc));

    if (rdb.isResourceTrue("cpu"))
        cpuFactory(rdb, kc, meters);

    if (rdb.isResourceTrue("mem"))
        meters.push_back(std::make_unique<MemMeter>(kc));

    if (rdb.isResourceTrue("disk"))
        meters.push_back(std::make_unique<DiskMeter>(kc));

    if (rdb.isResourceTrue("filesys"))
        util::concat(meters, FSMeterFactory().make(rdb));

    if (rdb.isResourceTrue("swap"))
        meters.push_back(std::make_unique<SwapMeter>());

    if (rdb.isResourceTrue("page"))
        meters.push_back(std::make_unique<PageMeter>(kc));

    if (rdb.isResourceTrue("net"))
        meters.push_back(std::make_unique<NetMeter>(kc));

    if (rdb.isResourceTrue("irqrate"))
        meters.push_back(std::make_unique<IrqRateMeter>(kc));

    return meters;
}


void MeterMaker::cpuFactory(const ResDB &rdb, kstat_ctl_t *kc,
  mlist &meters)  const {

    int cpuCount = sysconf(_SC_NPROCESSORS_ONLN);

    size_t start = 0, end = 0;
    getRange(rdb.getResource("cpuFormat"), cpuCount, start, end);

    KStatList *cpulist = KStatList::getList(kc, KStatList::CPU_STAT);
    for (size_t i = start ; i <= end ; i++)
        if (i == 0)
            meters.push_back(std::make_unique<CPUMeter>(kc, -1));
        else
            meters.push_back(std::make_unique<CPUMeter>(kc,
                (*cpulist)[i - 1]->ks_instance));
}
