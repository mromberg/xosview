//
//  Copyright (c) 1996 by Massimiliano Ghilardi ( ghilardi@cibs.sns.it )
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: pagemeter.h,v 1.3 2004/06/01 03:07:41 romberg Exp $
//
#ifndef _PAGEMETER_H_
#define _PAGEMETER_H_

#include "fieldmetergraph.h"

class PageMeter : public FieldMeterGraph {
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
  bool _vmstat;
  char *_statFileName;

  void getpageinfo( void );
  void getvmpageinfo( void );
  void updateinfo(void);
private:
};

#endif
