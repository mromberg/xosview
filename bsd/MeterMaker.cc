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



MeterMaker::MeterMaker(void) {
}


std::vector<Meter *> MeterMaker::makeMeters(const ResDB &rdb) {

    // Add the example meter.  Normally you would use
    // isResourceTrue.  But example resources are not in Xdefalts
    if (rdb.getResourceOrUseDefault("example", "False") == "True")
        _meters.push_back(new ExampleMeter());

    // Standard meters (usually added, but users could turn them off)
    if ( rdb.isResourceTrue("load") )
        _meters.push_back(new LoadMeter());

    if ( rdb.isResourceTrue("cpu") )
        cpuFactory(rdb);

    if ( rdb.isResourceTrue("mem") )
        _meters.push_back(new MemMeter());

    if (rdb.isResourceTrue("filesys"))
        util::concat(_meters, FSMeterFactory().make(rdb));

    if ( rdb.isResourceTrue("swap") )
        _meters.push_back(new SwapMeter());

    if ( rdb.isResourceTrue("page") )
        _meters.push_back(new PageMeter());

    if ( rdb.isResourceTrue("net") )
        _meters.push_back(new NetMeter());

    if ( rdb.isResourceTrue("disk") )
        _meters.push_back(new DiskMeter());

    if ( rdb.isResourceTrue("interrupts") )
        _meters.push_back(new IntMeter());

    if ( rdb.isResourceTrue("irqrate") )
        _meters.push_back(new IrqRateMeter());

    if ( rdb.isResourceTrue("battery") && BSDHasBattery() )
        _meters.push_back(new BtryMeter());

    if ( rdb.isResourceTrue("coretemp") )
        coreTempFactory(rdb);

    if ( rdb.isResourceTrue("bsdsensor") )
        sensorFactory(rdb);

    return _meters;
}


void MeterMaker::cpuFactory(const ResDB &rdb) {
    size_t start = 0, end = 0;
    getRange(rdb.getResource("cpuFormat"), CPUMeter::countCPUs(), start, end);

    logDebug << "start=" << start << ", end=" << end << std::endl;

    for (size_t i = start ; i <= end ; i++)
        _meters.push_back(new CPUMeter(i));
}


void MeterMaker::coreTempFactory(const ResDB &rdb) {
#if defined(__i386__) || defined(__x86_64__)
    if ( CoreTemp::countCpus() > 0 ) {
        std::string caption("ACT(\260C)/HIGH/");
        caption += rdb.getResourceOrUseDefault( "coretempHighest", "100" );
        std::string displayType = rdb.getResourceOrUseDefault(
            "coretempDisplayType", "separate");
        if (displayType == "separate") {
            std::string name("CPU");
            for (uint i = 0; i < CoreTemp::countCpus(); i++)
                _meters.push_back(new CoreTemp(name + util::repr(i),
                    caption, i));
        }
        else if (displayType == "average")
            _meters.push_back(new CoreTemp("CPU", caption, -1));
        else if (displayType == "maximum")
            _meters.push_back(new CoreTemp("CPU", caption, -2));
        else {
            logFatal << "Unknown value of coretempDisplayType: "
                     << displayType << std::endl;
        }
    }
#endif
}


void MeterMaker::sensorFactory(const ResDB &rdb) {
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
        _meters.push_back(new BSDSensor(name, high, low, label,
            caption, i));
    }
}
