//
//  Copyright (c) 1994, 1995, 2002, 2006, 2015, 2016, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "MeterMaker.h"
#include "cpumeter.h"
#include "memmeter.h"
#include "swapmeter.h"
#include "ppagemeter.h"
#include "wlinkmeter.h"
#include "pnetmeter.h"
#include "pintratemeter.h"
#include "intmeter.h"
#include "serialmeter.h"
#include "ploadmeter.h"
#include "btrymeter.h"
#include "pdiskmeter.h"
#include "raidmeter.h"
#include "tzonemeter.h"
#include "lmstemp.h"
#include "nfsmeter.h"
#include "cfsmeter.h"
#include "example.h"  // The example meter

#include <iomanip>


ComMeterMaker::mlist MeterMaker::makeMeters(const ResDB &rdb) {

    mlist meters;

    // Add the example meter.  Normally you would use
    // isResourceTrue.  But example resources are not in Xdefalts
    if (rdb.getResourceOrUseDefault("example", "False") == "True")
        meters.push_back(std::make_unique<ExampleMeter>());

    // Standard meters (usually added, but users could turn them off)
    if (rdb.isResourceTrue("load"))
        meters.push_back(std::make_unique<PrcLoadMeter>());

    if (rdb.isResourceTrue("cpu"))
        cpuFactory(rdb, meters);

    if (rdb.isResourceTrue("mem"))
        meters.push_back(std::make_unique<MemMeter>());

    if (rdb.isResourceTrue("disk"))
        meters.push_back(std::make_unique<PrcDiskMeter>());

    if (rdb.isResourceTrue("RAID")) {
        for (const auto &device : RAIDMeter::devices(rdb))
            meters.push_back(std::make_unique<RAIDMeter>(device));
    }

    if (rdb.isResourceTrue("filesys"))
        util::concat(meters, ComFSMeterFactory().make(rdb));

    if (rdb.isResourceTrue("swap"))
        meters.push_back(std::make_unique<SwapMeter>());

    if (rdb.isResourceTrue("page"))
        meters.push_back(std::make_unique<PrcPageMeter>());

    if (rdb.isResourceTrue("wlink"))
        meters.push_back(std::make_unique<WLinkMeter>());

    if (rdb.isResourceTrue("net"))
        meters.push_back(std::make_unique<PrcNetMeter>());

    if (rdb.isResourceTrue("NFSDStats"))
        meters.push_back(std::make_unique<NFSDStats>());

    if (rdb.isResourceTrue("NFSStats"))
        meters.push_back(std::make_unique<NFSStats>());

    // serial factory checks all resources.
    serialFactory(rdb, meters);

    if (rdb.isResourceTrue("irqrate"))
        meters.push_back(std::make_unique<PrcIrqRateMeter>());

    if (rdb.isResourceTrue("interrupts"))
        intFactory(rdb, meters);

    if (rdb.isResourceTrue("battery"))
        meters.push_back(std::make_unique<BtryMeter>());

    if (rdb.isResourceTrue("tzone"))
        tzoneFactory(meters);

    if (rdb.isResourceTrue("lmstemp"))
        lmsTempFactory(rdb, meters);

    return meters;
}


void MeterMaker::cpuFactory(const ResDB &rdb,
  std::vector<std::unique_ptr<Meter>> &meters) const {

    size_t start = 0, end = 0;
    getRange(rdb.getResource("cpuFormat"), CPUMeter::countCPUs(), start, end);

    logDebug << "start=" << start << ", end=" << end << std::endl;

    for (size_t i = start ; i <= end ; i++)
        meters.push_back(std::make_unique<CPUMeter>(i));
}


void MeterMaker::serialFactory(const ResDB &rdb, mlist  &meters) const {
// these architectures have no ioperm()
#if defined (__arm__) || defined(__mc68000__) || defined(__powerpc__) || defined(__sparc__) || defined(__s390__) || defined(__s390x__)
#else
    for (size_t i = 0 ; i < SerialMeter::numDevices() ; i++) {
        bool ok ;  unsigned long val ;
        std::string res = SerialMeter::getResourceName(
            (SerialMeter::Device)i);
        if ( !(ok = rdb.isResourceTrue(res)) ) {
            std::istringstream is(rdb.getResource(res));
            is >> std::setbase(0) >> val;
            if (!is)
                ok = false;
            else
                ok = val & 0xFFFF;
        }

        if (ok)
            meters.push_back(std::make_unique<SerialMeter>(
                  static_cast<SerialMeter::Device>(i)));
    }
#endif
}


void MeterMaker::intFactory(const ResDB &rdb, mlist &meters) const {
    size_t start = 0, end = 0;
    size_t cpuCount = CPUMeter::countCPUs();
    getRange(rdb.getResource("intFormat"), cpuCount, start, end);

    logDebug << "int range: " << start << ", " << end << std::endl;

    for (size_t i = start ; i <= end ; i++)
        meters.push_back(std::make_unique<IntMeter>(i));
}


void MeterMaker::lmsTempFactory(const ResDB &rdb, mlist &meters) const {
    std::string caption = "ACT/HIGH/"
        + rdb.getResourceOrUseDefault("lmstempHighest", "100");
    for (int i = 0 ; ; i++) {
        std::string istr = std::to_string(i);
        std::string res = rdb.getResourceOrUseDefault("lmstemp" + istr,
          "<nil>");
        if(res == "<nil>")
            break;

        std::string lab = rdb.getResourceOrUseDefault("lmstempLabel" + istr,
          "TMP");
        meters.push_back(std::make_unique<LmsTemp>(res, lab, caption));
    }
}


void MeterMaker::tzoneFactory(mlist &meters) const {
    const size_t nzones = TZoneMeter::count();

    if (!nzones)
        logProblem << "tzone enabled but no thermal zones found.\n";

    for (size_t i = 0 ; i < nzones ; i++)
        meters.push_back(std::make_unique<TZoneMeter>(i));
}
