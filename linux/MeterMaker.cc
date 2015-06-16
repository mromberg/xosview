//
//  Copyright (c) 1994, 1995, 2002, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "MeterMaker.h"
#include "xosview.h"

#include "cpumeter.h"
#include "memmeter.h"
#include "swapmeter.h"
#include "pagemeter.h"
#include "netmeter.h"
#include "intmeter.h"
#include "serialmeter.h"
#include "loadmeter.h"
#include "btrymeter.h"
#include "diskmeter.h"
#include "raidmeter.h"
#include "lmstemp.h"
#include "nfsmeter.h"

#include <stdlib.h>
#include <sstream>
#include <iomanip>

MeterMaker::MeterMaker(XOSView *xos){
    _xos = xos;
}

void MeterMaker::makeMeters(void){
    // check for the load meter
    if (_xos->isResourceTrue("load"))
        push(new LoadMeter(_xos));

    // Standard meters (usually added, but users could turn them off)
    if (_xos->isResourceTrue("cpu")){
        int cpuCount = CPUMeter::countCPUs();
        int start = (cpuCount == 0) ? 0 : 1;
        for (int i = start ; i <= cpuCount ; i++)
            push(new CPUMeter(_xos, CPUMeter::cpuStr(i)));
  }
    if (_xos->isResourceTrue("mem"))
        push(new MemMeter(_xos));
    if (_xos->isResourceTrue("disk"))
        push(new DiskMeter(_xos,
            util::stof(_xos->getResource("diskBandwidth"))));
    // check for the RAID meter
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

    // check for the net meter
    if (_xos->isResourceTrue("net"))
        push(new NetMeter(_xos, util::stof(_xos->getResource("netBandwidth"))));

    // check for the NFS mesters
    if (_xos->isResourceTrue("NFSDStats")){
        push(new NFSDStats(_xos));
    }
    if (_xos->isResourceTrue("NFSStats")){
        push(new NFSStats(_xos));
    }


    // check for the serial meters.
#if defined (__arm__) || defined(__mc68000__) || defined(__powerpc__) || defined(__sparc__) || defined(__s390__) || defined(__s390x__)
  /* these architectures have no ioperm() */
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

    // check for the interrupt meter
    if (_xos->isResourceTrue("interrupts")) {
        int cpuCount = IntMeter::countCPUs();
        cpuCount = cpuCount == 0 ? 1 : cpuCount;
        for (int i = 0 ; i < cpuCount ; i++)
            push(new IntMeter(_xos, i));
    }

    // check for the battery meter
    if (_xos->isResourceTrue("battery"))
        push(new BtryMeter(_xos));

    // check for the LmsTemp meter
    if (_xos->isResourceTrue("lmstemp")){
        std::string caption = "ACT/HIGH/"
            + _xos->getResourceOrUseDefault("lmstempHighest", "100");
        for (int i = 1 ; ; i++) {
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
}
