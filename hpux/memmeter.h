//  
//  Copyright (c) 1994, 1995 by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: memmeter.h,v 1.4 1999/01/23 22:20:40 mromberg Exp $
//
#ifndef _MEMMETER_H_
#define _MEMMETER_H_

#include "fieldmeterdecay.h"
#include <sys/pstat.h>

class MemMeter : public FieldMeterDecay {
public:
  MemMeter( XOSView *parent );
  ~MemMeter( void );

  const char *name( void ) const { return "MemMeter"; }  
  void checkevent( void );

  void checkResources(void);

protected:
  struct pst_status *stats_;
  int _pageSize;

  void getmeminfo( void );
private:
};


#endif
