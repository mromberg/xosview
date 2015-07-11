//
//  Copyright (c) 1994, 1995, 2002, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "MeterMaker.h"
#include "cpumeter.h"
#include "memmeter.h"
#include "swapmeter.h"
#include "pagemeter.h"
#include "netmeter.h"
#include "intratemeter.h"
#include "intmeter.h"
#include "serialmeter.h"
#include "loadmeter.h"
#include "btrymeter.h"
#include "diskmeter.h"
#include "raidmeter.h"
#include "lmstemp.h"
#include "nfsmeter.h"
#include "example.h"  // The example meter

#include <sstream>
#include <iomanip>

MeterMaker::MeterMaker(XOSView *xos){
    _xos = xos;
}

void MeterMaker::makeMeters(void){

    // Add the example meter.  Normally you would use
    // isResourceTrue.  But example resources are not in Xdefalts
    if (_xos->getResourceOrUseDefault("example", "False") == "True")
        push(new ExampleMeter(_xos));

    // Standard meters (usually added, but users could turn them off)
    if (_xos->isResourceTrue("load"))
        push(new LoadMeter(_xos));

    if (_xos->isResourceTrue("cpu"))
        cpuFactory();

    if (_xos->isResourceTrue("mem"))
        push(new MemMeter(_xos));

    if (_xos->isResourceTrue("disk"))
        push(new DiskMeter(_xos,
            util::stof(_xos->getResource("diskBandwidth"))));

    if (_xos->isResourceTrue("RAID")){
        int RAIDCount = util::stoi(_xos->getResource("RAIDdevicecount"));
        for (int i = 0 ; i < RAIDCount ; i++)
            push(new RAIDMeter(_xos, i));
    }

    if (_xos->isResourceTrue("swap"))
        push(new SwapMeter(_xos));

    if (_xos->isResourceTrue("page"))
        push(new PageMeter(_xos,
            util::stof(_xos->getResource("pageBandwidth"))));

    if (_xos->isResourceTrue("net"))
        push(new NetMeter(_xos));

    if (_xos->isResourceTrue("NFSDStats"))
        push(new NFSDStats(_xos));

    if (_xos->isResourceTrue("NFSStats"))
        push(new NFSStats(_xos));

    // serial factory checks all resources.
    serialFactory();

    if (_xos->isResourceTrue("irqrate"))
        push(new IrqRateMeter(_xos));

    if (_xos->isResourceTrue("interrupts"))
        intFactory();

    if (_xos->isResourceTrue("battery"))
        push(new BtryMeter(_xos));

    if (_xos->isResourceTrue("lmstemp"))
        lmsTempFactory();
}

void MeterMaker::getRange(const std::string &resource, size_t cpuCount,
  size_t &start, size_t &end) const {
    // check the *Format resource if multi-procesor system
    start = end = 0;

    if (cpuCount > 1) {
        std::string format(_xos->getResource(resource));
        logDebug << resource << ": " << format << std::endl;
        if (format == "single") // single meter for all cpus
            end = 0;
        else if (format == "all"){ // seperate but no cumulative
            start = 1;
            end = cpuCount;
        }
        else if (format == "both") // seperate + cumulative
            end = cpuCount;
        else if (format == "auto") // if(cpuCount==1) single else both
            end = cpuCount;
        else {
            logProblem << "Unknown " << resource << ": " << format << ".  "
                       << "Using auto" << std::endl;
            end = cpuCount;
        }
    }
}


void MeterMaker::cpuFactory(void) {
    size_t start = 0, end = 0;
    getRange("cpuFormat", CPUMeter::countCPUs(), start, end);

    logDebug << "start=" << start << ", end=" << end << std::endl;

    for (size_t i = start ; i <= end ; i++)
        push(new CPUMeter(_xos, i));
}

void MeterMaker::serialFactory(void) {
// these architectures have no ioperm()
#if defined (__arm__) || defined(__mc68000__) || defined(__powerpc__) || defined(__sparc__) || defined(__s390__) || defined(__s390x__)
#else
    for (int i = 0 ; i < SerialMeter::numDevices() ; i++) {
        bool ok ;  unsigned long val ;
        const char *res = SerialMeter::getResourceName((SerialMeter::Device)i);
        if ( !(ok = _xos->isResourceTrue(res)) ) {
            std::istringstream is(_xos->getResource(res));
            is >> std::setbase(0) >> val;
            if (!is)
                ok = false;
            else
                ok = val & 0xFFFF;
        }

        if ( ok )
            push(new SerialMeter(_xos, (SerialMeter::Device)i));
    }
#endif
}

void MeterMaker::intFactory(void) {
    size_t start = 0, end = 0;
    size_t cpuCount = CPUMeter::countCPUs();
    getRange("intFormat", cpuCount, start, end);

    logDebug << "int range: " << start << ", " << end << std::endl;

    for (size_t i = start ; i <= end ; i++)
        push(new IntMeter(_xos, i, cpuCount));
}

void MeterMaker::lmsTempFactory(void) {
    std::string caption = "ACT/HIGH/"
        + _xos->getResourceOrUseDefault("lmstempHighest", "100");
    for (int i = 0 ; ; i++) {
        std::ostringstream s;
        s << "lmstemp" << i;
        std::string res = _xos->getResourceOrUseDefault(s.str(), "<nil>");
        if(res == "<nil>")
            break;
        std::ostringstream s2;
        s2 << "lmstempLabel" << i;
        std::string lab = _xos->getResourceOrUseDefault(s2.str(), "TMP");
        push(new LmsTemp(_xos, res, lab, caption));
    }
}
