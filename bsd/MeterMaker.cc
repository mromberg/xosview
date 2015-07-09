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
// $Id: MeterMaker.cc,v 1.20 2002/03/22 03:23:40 bgrayson Exp $
//
#include <stdlib.h>
#include "general.h"
#include "MeterMaker.h"
#include "xosview.h"
#include "strutil.h"



#include "cpumeter.h"

// copied from linux
#include "memmeter.h"
#include "swapmeter.h"
#include "pagemeter.h"

// function with the old code.  But ya gotta be root.
#include "netmeter.h"
#include "loadmeter.h"

// Old code is broken (looks like kernel changes)
// prospects to use /proc are not good.
//#include "diskmeter.h"
//#include "intmeter.h"

// Can probably work from /proc
//#include "intratemeter.h"

// No clue but bets are it is non functional.
#ifdef HAVE_BATTERY_METER
#include "btrymeter.h"
#endif


MeterMaker::MeterMaker(XOSView *xos){
  _xos = xos;
}

void MeterMaker::makeMeters(void){
  //  check for the loadmeter
  if (_xos->isResourceTrue("load"))
    push(new LoadMeter(_xos));

  // Standard meters (usually added, but users could turn them off)
  if (_xos->isResourceTrue("cpu"))
    push(new CPUMeter(_xos));

  if (_xos->isResourceTrue("mem"))
      push(new MemMeter(_xos));

  if (_xos->isResourceTrue("swap"))
    push(new SwapMeter(_xos));

  if (_xos->isResourceTrue("page"))
      push(new PageMeter (_xos, util::stof(_xos->getResource("pageBandwidth")))
          );

  // check for the net meter
  if (_xos->isResourceTrue("net"))
      push(new NetMeter(_xos, util::stof(_xos->getResource("netBandwidth"))));

  // if (_xos->isResourceTrue("disk"))
  //     push(new DiskMeter (_xos, util::stof(_xos->getResource("diskBandwidth"))));

//  if (_xos->isResourceTrue("interrupts"))
//      push(new IntMeter(_xos));

  // if (_xos->isResourceTrue("irqrate"))
  // {
  //     push(new IrqRateMeter(_xos));
  // }

#ifdef HAVE_BATTERY_METER
  //  NOTE:  Only xosview for NetBSD (out of all the BSDs) currently
  //  supports the Battery Meter.
  //  This one is done in its own file, not kernel.cc
  if (_xos->isResourceTrue("battery"))
      push(new BtryMeter(_xos));
#endif

  //  The serial meters are not yet available for the BSDs.  BCG
}
