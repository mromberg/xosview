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
// $Id: netmeter.cc,v 1.14 1998/10/20 19:37:34 bgrayson Exp $
//
#include <stdlib.h>		//  For atoi().  BCG
#include "general.h"
#include "netmeter.h"
#include "kernel.h"

CVSID("$Id: netmeter.cc,v 1.14 1998/10/20 19:37:34 bgrayson Exp $");
CVSID_DOT_H(NETMETER_H_CVSID);
CVSID_DOT_H2(TIMER_H_CVSID);
CVSID_DOT_H3(TIMEVAL_H_CVSID);

NetMeter::NetMeter( XOSView *parent, float max )
  : FieldMeterGraph( parent, 3, "NET", "IN/OUT/IDLE" ){
  IntervalTimerStart();
  netBandwidth_ = max;
  total_ = netBandwidth_;
  _lastBytesIn = _lastBytesOut = 0;
  BSDGetNetInOut (&_lastBytesIn, &_lastBytesOut);
}

NetMeter::~NetMeter( void ){
}

void NetMeter::checkResources( void ){
  FieldMeter::checkResources();

  setfieldcolor( 0, parent_->getResource("netInColor") );
  setfieldcolor( 1, parent_->getResource("netOutColor") );
  setfieldcolor( 2, parent_->getResource("netBackground") );
  priority_ = atoi (parent_->getResource("netPriority") );
  dodecay_ = !strncasecmp (parent_->getResource("netDecay"),"True", 5);
  useGraph_ = !strncasecmp (parent_->getResource("netGraph"),"True", 5);
  SetUsedFormat (parent_->getResource("netUsedFormat"));
}

void NetMeter::checkevent( void ){
  IntervalTimerStop();

//  Reset total_ to expected maximum.  If it is too low, it
//  will be adjusted in adjust().  bgrayson
  total_ = netBandwidth_;

  fields_[0] = fields_[1] = 0;

//  Begin NetBSD-specific code.  BCG
  long long nowBytesIn, nowBytesOut;

//  The BSDGetNetInOut() function is in kernel.cc    BCG
  BSDGetNetInOut (&nowBytesIn, &nowBytesOut);
  long long correction = 0x10000000;
  correction *= 0x10;
  /*  Deal with 32-bit wrap by making last value 2^32 less.  Yes,
   *  this is a better idea than adding to nowBytesIn -- the
   *  latter would only work for the first wrap (1+2^32 vs. 1)
   *  but not for the second (1+2*2^32 vs. 1) -- 1+2^32 -
   *  (1+2^32) is still too big.  */
  if (nowBytesIn < _lastBytesIn) _lastBytesIn -= correction;
  if (nowBytesOut < _lastBytesOut) _lastBytesOut -= correction;
  float t = (1e6) / IntervalTimeInMicrosecs();
  fields_[0] = (float)(nowBytesIn - _lastBytesIn) * t;
  _lastBytesIn = nowBytesIn;
  fields_[1] = (float)(nowBytesOut - _lastBytesOut) * t;
  _lastBytesOut = nowBytesOut;
//  End BSD-specific code.  BCG

  adjust();
  fields_[2] = total_ - fields_[0] - fields_[1];
    /*  The fields_ values have already been scaled into bytes/sec by
     *  the manipulations (* t) above.  */
  setUsed (fields_[0]+fields_[1], total_);
  IntervalTimerStart();
  drawfields();
}

void NetMeter::adjust(void){
  if (total_ < (fields_[0] + fields_[1]))
    total_ = fields_[0] + fields_[1];
}
