//  
//  Copyright (c) 1997 by Mike Romberg (romberg@fsl.noaa.gov)
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: pagemeter.h,v 1.1 1997/02/26 18:34:57 mromberg Exp $
//
#ifndef _PAGEMETER_H_
#define _PAGEMETER_H_

#include "fieldmeterdecay.h"

class PageMeter : public FieldMeterDecay {
public:
  PageMeter( XOSView *parent, float max );
  ~PageMeter( void );

  const char *name( void ) const { return "PageMeter"; }
  void checkevent( void );

  void checkResources( void );
protected:
  float pageinfo_[2][2];
  int pageindex_;
  float maxspeed_;

  void getpageinfo( void );
private:
};

#endif
