//  
//  Copyright (c) 1994, 1995 by Mike Romberg ( romberg@fsl.noaa.gov )
//  Copyright (c) 1995, 1996, 1997 by Brian Grayson (bgrayson@ece.utexas.edu)
//
//  This file was written by Brian Grayson for the NetBSD and xosview
//    projects.
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//
// $Id: MeterMaker.cc,v 1.8 1997/08/09 01:31:08 bgrayson Exp $
//
#include <stdlib.h>
#include "general.h"
#include "MeterMaker.h"
#include "xosview.h"
#include "cpumeter.h"
#include "memmeter.h"
#ifndef XOSVIEW_FREEBSD		/*  FreeBSD swap meter isn't done yet.  */
# include "swapmeter.h"
#endif
#include "netmeter.h"
#include "loadmeter.h"
#include "diskmeter.h"
#include "pagemeter.h"
//#include "intmeter.h"  //  These two are not yet supported under NetBSD.
//#include "serialmeter.h"

CVSID_DOT_H2(PLLIST_H_CVSID);
CVSID_DOT_H(METERMAKER_H_CVSID);
CVSID("$Id: MeterMaker.cc,v 1.8 1997/08/09 01:31:08 bgrayson Exp $");

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
#ifndef XOSVIEW_FREEBSD		/*  FreeBSD swap meter isn't done yet.  */
  if (_xos->isResourceTrue("swap"))
    push(new SwapMeter(_xos));
#endif

  // check for the net meter
  if (_xos->isResourceTrue("net"))
    push(new NetMeter(_xos, atof(_xos->getResource("netBandwidth"))));

#ifndef XOSVIEW_FREEBSD		/*  FreeBSD diskmeter also isn't done.  */
  if (_xos->isResourceTrue("disk"))
    push(new DiskMeter (_xos, atof(_xos->getResource("diskBandwidth"))));
#endif

  if (_xos->isResourceTrue("page"))
    push(new PageMeter (_xos, atof(_xos->getResource("pageBandwidth"))));

  //  The serial meters and the interrupt meter are not yet
  //  available for NetBSD.  BCG
}
