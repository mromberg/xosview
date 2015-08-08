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
#include "wlinkmeter.h"
#include "netmeter.h"
#include "intratemeter.h"
#include "intmeter.h"
#include "serialmeter.h"
#include "loadmeter.h"
#include "btrymeter.h"
#include "diskmeter.h"
#include "raidmeter.h"
#include "tzonemeter.h"
#include "lmstemp.h"
#include "nfsmeter.h"
#include "fsmeter.h"
#include "example.h"  // The example meter

#include <sstream>
#include <iomanip>

MeterMaker::MeterMaker(XOSView *xos){
    _xos = xos;
}

void MeterMaker::makeMeters(const ResDB &rdb){

    // Add the example meter.  Normally you would use
    // isResourceTrue.  But example resources are not in Xdefalts
    if (rdb.getResourceOrUseDefault("example", "False") == "True")
        push(new ExampleMeter(_xos));

    // Standard meters (usually added, but users could turn them off)
    if (rdb.isResourceTrue("load"))
        push(new LoadMeter(_xos));

    if (rdb.isResourceTrue("cpu"))
        cpuFactory(rdb);

    if (rdb.isResourceTrue("mem"))
        push(new MemMeter(_xos));

    if (rdb.isResourceTrue("disk"))
        push(new DiskMeter(_xos,
            util::stof(rdb.getResource("diskBandwidth"))));

    if (rdb.isResourceTrue("RAID")){
        int RAIDCount = util::stoi(rdb.getResource("RAIDdevicecount"));
        for (int i = 0 ; i < RAIDCount ; i++)
            push(new RAIDMeter(_xos, i));
    }

    if (rdb.isResourceTrue("filesys")) {
        std::vector<std::string> fs = FSMeter::mounts(rdb);
        for (size_t i = 0 ; i < fs.size() ; i++)
            push(new FSMeter(_xos, fs[i]));
    }

    if (rdb.isResourceTrue("swap"))
        push(new SwapMeter(_xos));

    if (rdb.isResourceTrue("page"))
        push(new PageMeter(_xos,
            util::stof(rdb.getResource("pageBandwidth"))));

    if (rdb.isResourceTrue("wlink"))
        push(new WLinkMeter(_xos));

    if (rdb.isResourceTrue("net"))
        push(new NetMeter(_xos));

    if (rdb.isResourceTrue("NFSDStats"))
        push(new NFSDStats(_xos));

    if (rdb.isResourceTrue("NFSStats"))
        push(new NFSStats(_xos));

    // serial factory checks all resources.
    serialFactory(rdb);

    if (rdb.isResourceTrue("irqrate"))
        push(new IrqRateMeter(_xos));

    if (rdb.isResourceTrue("interrupts"))
        intFactory(rdb);

    if (rdb.isResourceTrue("battery"))
        push(new BtryMeter(_xos));

    if (rdb.isResourceTrue("tzone"))
        tzoneFactory();

    if (rdb.isResourceTrue("lmstemp"))
        lmsTempFactory(rdb);
}

void MeterMaker::getRange(const ResDB &rdb, const std::string &resource,
  size_t cpuCount, size_t &start, size_t &end) const {

    // check the *Format resource if multi-procesor system
    start = end = 0;

    if (cpuCount > 1) {
        std::string format(rdb.getResource(resource));
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


void MeterMaker::cpuFactory(const ResDB &rdb) {
    size_t start = 0, end = 0;
    getRange(rdb, "cpuFormat", CPUMeter::countCPUs(), start, end);

    logDebug << "start=" << start << ", end=" << end << std::endl;

    for (size_t i = start ; i <= end ; i++)
        push(new CPUMeter(_xos, i));
}

void MeterMaker::serialFactory(const ResDB &rdb) {
// these architectures have no ioperm()
#if defined (__arm__) || defined(__mc68000__) || defined(__powerpc__) || defined(__sparc__) || defined(__s390__) || defined(__s390x__)
#else
    for (int i = 0 ; i < SerialMeter::numDevices() ; i++) {
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
            push(new SerialMeter(_xos, (SerialMeter::Device)i));
    }
#endif
}

void MeterMaker::intFactory(const ResDB &rdb) {
    size_t start = 0, end = 0;
    size_t cpuCount = CPUMeter::countCPUs();
    getRange(rdb, "intFormat", cpuCount, start, end);

    logDebug << "int range: " << start << ", " << end << std::endl;

    for (size_t i = start ; i <= end ; i++)
        push(new IntMeter(_xos, i, cpuCount));
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
        push(new LmsTemp(_xos, res, lab, caption));
    }
}

void MeterMaker::tzoneFactory(void) {
    size_t nzones = TZoneMeter::count();

    if (!nzones)
        logProblem << "tzone enabled but no thermal zones found.\n";

    for (size_t i = 0 ; i < nzones ; i++)
        push(new TZoneMeter(_xos, i));
}
