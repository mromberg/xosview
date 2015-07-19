//  
//  Copyright (c) 1997 by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  This file may be distributed under terms of the GPL
//  Ported to NetBSD by David W. Talmage
//  (talmage@jefferson.cmf.nrl.navy.mil)
//
//
// $Id: btrymeter.h,v 1.1 1999/01/25 20:14:33 bgrayson Exp $
//
#ifndef _BTRYMETER_H_
#define _BTRYMETER_H_


#include "fieldmeter.h"


class BtryMeter : public FieldMeter {
public:
  BtryMeter( XOSView *parent );
  ~BtryMeter( void );

  const char *name( void ) const { return "BtryMeter"; }  
  void checkevent( void );

  void checkResources( void );
protected:

  void getpwrinfo( void );
private:
  int alarmThreshold;
};


#endif
