//  
//  Copyright (c) 1994, 1995 by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  NetBSD port:  
//  Copyright (c) 1995 Brian Grayson(bgrayson@pine.ece.utexas.edu)
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: cpumeter.h,v 1.4 1996/11/24 04:38:29 bgrayson Exp $
//
#ifndef _CPUMETER_H_
#define _CPUMETER_H_

#define CPUMETER_H_CVSID "$Id: "

#include "fieldmeterdecay.h"

class CPUMeter : public FieldMeterDecay {
public:
  CPUMeter( XOSView *parent );
  ~CPUMeter( void );

  const char *name( void ) const { return "CPUMeter"; }
  void checkevent( void );

  void checkResources( void );
protected:
  float cputime_[2][4];
  int cpuindex_;

  void getcputime( void );
private:
};

#endif
