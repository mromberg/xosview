//  
//  Copyright (c) 1996 by Massimiliano Ghilardi ( ghilardi@cibs.sns.it )
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: pagemeter.h,v 1.1 1996/10/27 23:23:03 mromberg Exp $
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
