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


std::vector<Meter *> MeterMaker::makeMeters(const ResDB &rdb) {

    // Add the example meter.  Normally you would use
    // isResourceTrue.  But example resources are not in Xdefalts
    if (rdb.getResourceOrUseDefault("example", "False") == "True")
        _meters.push_back(new ExampleMeter());

    // Standard meters (usually added, but users could turn them off)
    if (rdb.isResourceTrue("load"))
        _meters.push_back(new PrcLoadMeter());

    if (rdb.isResourceTrue("cpu"))
        cpuFactory(rdb);

    if (rdb.isResourceTrue("mem"))
        _meters.push_back(new MemMeter());

    if (rdb.isResourceTrue("disk"))
        _meters.push_back(new PrcDiskMeter());

    if (rdb.isResourceTrue("RAID")){
        std::vector<std::string> devices(RAIDMeter::devices(rdb));
        for (size_t i = 0 ; i < devices.size() ; i ++)
            _meters.push_back(new RAIDMeter(devices[i]));
    }

    if (rdb.isResourceTrue("filesys"))
        util::concat(_meters, ComFSMeterFactory().make(rdb));

    if (rdb.isResourceTrue("swap"))
        _meters.push_back(new SwapMeter());

    if (rdb.isResourceTrue("page"))
        _meters.push_back(new PrcPageMeter());

    if (rdb.isResourceTrue("wlink"))
        _meters.push_back(new WLinkMeter());

    if (rdb.isResourceTrue("net"))
        _meters.push_back(new PrcNetMeter());

    if (rdb.isResourceTrue("NFSDStats"))
        _meters.push_back(new NFSDStats());

    if (rdb.isResourceTrue("NFSStats"))
        _meters.push_back(new NFSStats());

    // serial factory checks all resources.
    serialFactory(rdb);

    if (rdb.isResourceTrue("irqrate"))
        _meters.push_back(new PrcIrqRateMeter());

    if (rdb.isResourceTrue("interrupts"))
        intFactory(rdb);

    if (rdb.isResourceTrue("battery"))
        _meters.push_back(new BtryMeter());

    if (rdb.isResourceTrue("tzone"))
        tzoneFactory();

    if (rdb.isResourceTrue("lmstemp"))
        lmsTempFactory(rdb);

    return _meters;
}


void MeterMaker::cpuFactory(const ResDB &rdb) {
    size_t start = 0, end = 0;
    getRange(rdb.getResource("cpuFormat"), CPUMeter::countCPUs(), start, end);

    logDebug << "start=" << start << ", end=" << end << std::endl;

    for (size_t i = start ; i <= end ; i++)
        _meters.push_back(new CPUMeter(i));
}


void MeterMaker::serialFactory(const ResDB &rdb) {
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

        if ( ok )
            _meters.push_back(new SerialMeter((SerialMeter::Device)i));
    }
#endif
}


void MeterMaker::intFactory(const ResDB &rdb) {
    size_t start = 0, end = 0;
    size_t cpuCount = CPUMeter::countCPUs();
    getRange(rdb.getResource("intFormat"), cpuCount, start, end);

    logDebug << "int range: " << start << ", " << end << std::endl;

    for (size_t i = start ; i <= end ; i++)
        _meters.push_back(new IntMeter(i));
}


void MeterMaker::lmsTempFactory(const ResDB &rdb) {
    std::string caption = "ACT/HIGH/"
        + rdb.getResourceOrUseDefault("lmstempHighest", "100");
    for (int i = 0 ; ; i++) {
        std::ostringstream s;
        s << "lmstemp" << i;
        std::string res = rdb.getResourceOrUseDefault(s.str(), "<nil>");
        if(res == "<nil>")
            break;
        std::ostringstream s2;
        s2 << "lmstempLabel" << i;
        std::string lab = rdb.getResourceOrUseDefault(s2.str(), "TMP");
        _meters.push_back(new LmsTemp(res, lab, caption));
    }
}


void MeterMaker::tzoneFactory(void) {
    size_t nzones = TZoneMeter::count();

    if (!nzones)
        logProblem << "tzone enabled but no thermal zones found.\n";

    for (size_t i = 0 ; i < nzones ; i++)
        _meters.push_back(new TZoneMeter(i));
}
