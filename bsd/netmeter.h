//  
//  Copyright (c) 1994, 1995 by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  NetBSD port:  
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
// $Id: netmeter.h,v 1.10 1998/10/20 19:37:34 bgrayson Exp $
//
#ifndef _NETMETER_H_
#define _NETMETER_H_

#define NETMETER_H_CVSID "$Id: netmeter.h,v 1.10 1998/10/20 19:37:34 bgrayson Exp $"

#include "fieldmetergraph.h"
#include "timer.h"

class Host;

class NetMeter : public FieldMeterGraph {
public:
  NetMeter(XOSView *parent, float max);
  ~NetMeter( void );

  const char *name( void ) const { return "NetMeter"; }  
  void checkevent( void );

  void checkResources( void );
protected:
  float netBandwidth_;

private:
  //  NetBSD:  Use long long, so we won't run into problems after 4 GB
  //  has been transferred over the net!
  long long _lastBytesIn, _lastBytesOut;

  void adjust(void);
};

#endif
