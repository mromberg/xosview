//
//  Copyright (c) 1994, 1995, 2015 by Mike Romberg ( romberg@fsl.noaa.gov )
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
#include "defines.h"
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
#include "sensor.h"

#if defined(__i386__) || defined(__x86_64__)
#include "coretemp.h"
#endif



MeterMaker::MeterMaker(XOSView *xos) : _xos(xos) {
}

void MeterMaker::makeMeters(void) {

    // Check the kernelName resource
    XOSView::opt kname(_xos->getOptResource("kernelName"));
    if (kname.first)
        SetKernelName(kname.second.c_str());

    // Standard meters (usually added, but users could turn them off)
    if ( _xos->isResourceTrue("load") )
        push(new LoadMeter(_xos));

    if ( _xos->isResourceTrue("cpu") )
        cpuFactory();

    if ( _xos->isResourceTrue("mem") )
        push(new MemMeter(_xos));

    if ( _xos->isResourceTrue("swap") )
        push(new SwapMeter(_xos));

    if ( _xos->isResourceTrue("page") )
        push(new PageMeter(_xos, util::stof(_xos->getResource("pageBandwidth"))));

    if ( _xos->isResourceTrue("net") )
        push(new NetMeter(_xos, util::stof(_xos->getResource("netBandwidth"))));

    if ( _xos->isResourceTrue("disk") )
        push(new DiskMeter(_xos, util::stof(_xos->getResource("diskBandwidth"))));

    if ( _xos->isResourceTrue("interrupts") )
        push(new IntMeter(_xos));

    if ( _xos->isResourceTrue("irqrate") )
        push(new IrqRateMeter(_xos));

    if ( _xos->isResourceTrue("battery") && BSDHasBattery() )
        push(new BtryMeter(_xos));

    if ( _xos->isResourceTrue("coretemp") )
        coreTempFactory();

    if ( _xos->isResourceTrue("bsdsensor") )
        sensorFactory();
}


void MeterMaker::cpuFactory(void) {
    bool single, both, all;
    unsigned int cpuCount = BSDCountCpus();

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
        push(new CPUMeter(_xos, 0));

    if (all || both) {
        for (unsigned int i = 1; i <= cpuCount; i++)
            push(new CPUMeter(_xos, i));
    }
}


void MeterMaker::coreTempFactory(void) {
#if defined(__i386__) || defined(__x86_64__)
    if ( CoreTemp::countCpus() > 0 ) {
        std::string caption("ACT(\260C)/HIGH/");
        caption += _xos->getResourceOrUseDefault( "coretempHighest", "100" );
        std::string displayType = _xos->getResourceOrUseDefault(
            "coretempDisplayType", "separate");
        if (displayType == "separate") {
            std::string name("CPU");
            for (uint i = 0; i < CoreTemp::countCpus(); i++)
                push(new CoreTemp(_xos, name + util::repr(i), caption, i));
        }
        else if (displayType == "average")
            push(new CoreTemp(_xos, "CPU", caption, -1));
        else if (displayType == "maximum")
            push(new CoreTemp(_xos, "CPU", caption, -2));
        else {
            logFatal << "Unknown value of coretempDisplayType: "
                     << displayType << std::endl;
        }
    }
#endif
}


void MeterMaker::sensorFactory(void) {
    std::string s, caption, l;
    for (int i = 1 ; ; i++) {
        s = "bsdsensorHighest" + util::repr(i);
        float highest = util::stof( _xos->getResourceOrUseDefault(s,
            "100") );
        caption = "ACT/HIGH/" + util::repr(highest);
        s = "bsdsensor" + util::repr(i);
        std::string name = _xos->getResourceOrUseDefault(s, "");
        if (!name.size())
            break;
        s = "bsdsensorHigh" + util::repr(i);
        std::string high = _xos->getResourceOrUseDefault(s, "");
        s = "bsdsensorLow" + util::repr(i);
        std::string low = _xos->getResourceOrUseDefault(s, "");
        s = "bsdsensorLabel" + util::repr(i);
        l = "SEN" + util::repr(i);
        std::string label = _xos->getResourceOrUseDefault(s, l);
        push(new BSDSensor(_xos, name, high, low, label, caption, i));
    }
}
