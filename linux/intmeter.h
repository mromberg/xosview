//
//  Copyright (c) 1994, 1995, 2006 by Mike Romberg ( mike.romberg@noaa.gov )
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: intmeter.h,v 1.7 2006/02/18 04:33:06 romberg Exp $
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
  unsigned long *irqs_, *lastirqs_;

  int _cpu;
  bool _old;

  void getirqs( void );
  void updateirqcount( int n, bool init );
  void initirqcount( void );
};

#endif
