//  
//  Copyright (c) 1994, 1995 by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  NetBSD port:  
//  Copyright (c) 1995 Brian Grayson(bgrayson@pine.ece.utexas.edu)
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: swapmeter.h,v 1.5 1997/02/14 05:42:13 bgrayson Exp $
//
#ifndef _SWAPMETER_H_
#define _SWAPMETER_H_

#define SWAPMETER_H_CVSID "$Id: swapmeter.h,v 1.5 1997/02/14 05:42:13 bgrayson Exp $"

#include "fieldmeterdecay.h"


class SwapMeter : public FieldMeterDecay {
public:
  SwapMeter( XOSView *parent );
  ~SwapMeter( void );

  const char *name( void ) const { return "SwapMeter"; }  
  void checkevent( void );

  void checkResources( void );
protected:

  void getswapinfo( void );
private:
};


#endif
