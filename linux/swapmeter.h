//
//  Copyright (c) 1994, 1995, 2006, 2015 by Mike Romberg ( mike.romberg@noaa.gov )
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: swapmeter.h,v 1.4 2006/02/18 04:33:06 romberg Exp $
//
#ifndef _SWAPMETER_H_
#define _SWAPMETER_H_


#include "fieldmetergraph.h"


class SwapMeter : public FieldMeterGraph {
public:
  SwapMeter( XOSView *parent );
  ~SwapMeter( void );

  std::string name( void ) const { return "SwapMeter"; }
  void checkevent( void );

  void checkResources( void );
protected:

  void getswapinfo( void );
private:
};


#endif
