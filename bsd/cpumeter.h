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
// $Id: cpumeter.h,v 1.8 1998/02/09 11:44:40 bgrayson Exp $
//
#ifndef _CPUMETER_H_
#define _CPUMETER_H_

#define CPUMETER_H_CVSID "$Id: cpumeter.h,v 1.8 1998/02/09 11:44:40 bgrayson Exp $"

#include "fieldmeterdecay.h"

class CPUMeter : public FieldMeterDecay {
public:
  CPUMeter( XOSView *parent );
  ~CPUMeter( void );

  const char *name( void ) const { return "CPUMeter"; }
  void checkevent( void );

  void checkResources( void );
protected:
  float cputime_[2][5];
  int cpuindex_;

  void getcputime( void );
private:
};

#endif
