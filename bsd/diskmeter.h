//  
//  NetBSD port:  
//  Copyright (c) 1995 Brian Grayson(bgrayson@pine.ece.utexas.edu)
//
//  This file may be distributed under terms of the GPL or of the BSD
//    copyright, whichever you choose.
//
//
// $Id: diskmeter.h,v 1.4 1997/02/14 05:41:54 bgrayson Exp $
//
#ifndef _DISKMETER_H_
#define _DISKMETER_H_

#define DISKMETER_H_CVSID "$Id: diskmeter.h,v 1.4 1997/02/14 05:41:54 bgrayson Exp $"

#include "fieldmeterdecay.h"
#include <sys/types.h>		//  For u_int64_t

class DiskMeter : public FieldMeterDecay {
public:
  DiskMeter( XOSView *parent, float max );
  ~DiskMeter( void );

  const char *name( void ) const { return "DiskMeter"; }
  void checkevent( void );

  void checkResources( void );
protected:
  void getstats( void );
private:
  u_int64_t prevBytes;
  int kernelHasStats_;
  float	maxBandwidth_;
};

#endif
