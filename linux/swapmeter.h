//  
//  Copyright (c) 1994, 1995 by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: swapmeter.h,v 1.3 1998/10/15 21:28:18 mromberg Exp $
//
#ifndef _SWAPMETER_H_
#define _SWAPMETER_H_


#include "fieldmetergraph.h"


class SwapMeter : public FieldMeterGraph {
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
