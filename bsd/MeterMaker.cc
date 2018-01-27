//
//  Copyright (c) 1994, 1995, 2015, 2016, 2018 by Mike Romberg ( romberg@fsl.noaa.gov )
//  Copyright (c) 1995, 1996, 1997-2002 by Brian Grayson (bgrayson@netbsd.org)
//
//  This file was written by Brian Grayson for the NetBSD and xosview
//    projects.
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//

#include "MeterMaker.h"
#include "kernel.h"

#include "loadmeter.h"
#include "cpumeter.h"
#include "memmeter.h"
#include "swapmeter.h"
#include "pagemeter.h"
#include "netmeter.h"
#include "diskmeter.h"
#include "intmeter.h"
#include "intratemeter.h"
#include "btrymeter.h"
#include "fsmeter.h"
#include "sensor.h"
#include "example.h"  // The example meter

#if defined(__i386__) || defined(__x86_64__)
#include "coretemp.h"
#endif



ComMeterMaker::mlist MeterMaker::makeMeters(const ResDB &rdb) {

    mlist meters;

    // Add the example meter.  Normally you would use
    // isResourceTrue.  But example resources are not in Xdefalts
    if (rdb.getResourceOrUseDefault("example", "False") == "True")
        meters.push_back(std::make_unique<ExampleMeter>());

    // Standard meters (usually added, but users could turn them off)
    if ( rdb.isResourceTrue("load") )
        meters.push_back(std::make_unique<LoadMeter>());

    if ( rdb.isResourceTrue("cpu") )
        cpuFactory(rdb, meters);

    if ( rdb.isResourceTrue("mem") )
        meters.push_back(std::make_unique<MemMeter>());

    if (rdb.isResourceTrue("filesys"))
        util::concat(meters, FSMeterFactory().make(rdb));

    if ( rdb.isResourceTrue("swap") )
        meters.push_back(std::make_unique<SwapMeter>());

    if ( rdb.isResourceTrue("page") )
        meters.push_back(std::make_unique<PageMeter>());

    if ( rdb.isResourceTrue("net") )
        meters.push_back(std::make_unique<NetMeter>());

    if ( rdb.isResourceTrue("disk") )
        meters.push_back(std::make_unique<DiskMeter>());

    if ( rdb.isResourceTrue("interrupts") )
        meters.push_back(std::make_unique<IntMeter>());

    if ( rdb.isResourceTrue("irqrate") )
        meters.push_back(std::make_unique<IrqRateMeter>());

    if ( rdb.isResourceTrue("battery") && BSDHasBattery() )
        meters.push_back(std::make_unique<BtryMeter>());

    if ( rdb.isResourceTrue("coretemp") )
        coreTempFactory(rdb, meters);

    if ( rdb.isResourceTrue("bsdsensor") )
        sensorFactory(rdb, meters);

    return meters;
}


void MeterMaker::cpuFactory(const ResDB &rdb, mlist &meters) const {
    size_t start = 0, end = 0;
    getRange(rdb.getResource("cpuFormat"), CPUMeter::countCPUs(), start, end);

    logDebug << "start=" << start << ", end=" << end << std::endl;

    for (size_t i = start ; i <= end ; i++)
        meters.push_back(std::make_unique<CPUMeter>(i));
}


void MeterMaker::coreTempFactory(const ResDB &rdb, mlist &meters) const {
#if defined(__i386__) || defined(__x86_64__)
    if ( CoreTemp::countCpus() > 0 ) {
        std::string caption("ACT(\260C)/HIGH/");
        caption += rdb.getResourceOrUseDefault( "coretempHighest", "100" );
        std::string displayType = rdb.getResourceOrUseDefault(
            "coretempDisplayType", "separate");
        if (displayType == "separate") {
            std::string name("CPU");
            for (uint i = 0; i < CoreTemp::countCpus(); i++)
                meters.push_back(std::make_unique<CoreTemp>(name
                    + std::to_string(i), caption, i));
        }
        else if (displayType == "average")
            meters.push_back(std::make_unique<CoreTemp>("CPU", caption, -1));
        else if (displayType == "maximum")
            meters.push_back(std::make_unique<CoreTemp>("CPU", caption, -2));
        else {
            logFatal << "Unknown value of coretempDisplayType: "
                     << displayType << std::endl;
        }
    }
#endif
}


void MeterMaker::sensorFactory(const ResDB &rdb, mlist &meters) const {
    std::string s, caption, l;
    for (int i = 1 ; ; i++) {
        s = "bsdsensorHighest" + util::repr(i);
        float highest = std::stof( rdb.getResourceOrUseDefault(s,
            "100") );
        caption = "ACT/HIGH/" + util::repr(highest);
        s = "bsdsensor" + util::repr(i);
        std::string name = rdb.getResourceOrUseDefault(s, "");
        if (!name.size())
            break;
        s = "bsdsensorHigh" + util::repr(i);
        std::string high = rdb.getResourceOrUseDefault(s, "");
        s = "bsdsensorLow" + util::repr(i);
        std::string low = rdb.getResourceOrUseDefault(s, "");
        s = "bsdsensorLabel" + util::repr(i);
        l = "SEN" + util::repr(i);
        std::string label = rdb.getResourceOrUseDefault(s, l);
        meters.push_back(std::make_unique<BSDSensor>(name, high, low, label,
            caption, i));
    }
}
