//  
//  Copyright (c) 1994, 1995 by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: intmeter.h,v 1.4 1998/03/07 21:20:39 mromberg Exp $
//
#ifndef _INTMETER_H_
#define _INTMETER_H_

#include "bitmeter.h"

class IntMeter : public BitMeter {
public:
  IntMeter( XOSView *parent, int cpu = 0);
  ~IntMeter( void );

  void checkevent( void );

  void checkResources( void );

  static float getLinuxVersion(void);
  static int countCPUs(void);

protected:
  unsigned long irqs_[24], lastirqs_[24];
  int _cpu;
  bool _old;

  void getirqs( void );
};

#endif
