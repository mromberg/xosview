//  
//  Copyright (c) 1994, 1995 by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: MeterMaker.h,v 1.2 1996/08/14 06:20:38 mromberg Exp $
//
#ifndef _MeterMaker_h
#define _MeterMaker_h

#include "pllist.h"

class Meter;
class XOSView;

class MeterMaker : public PLList<Meter *> {
public:
  MeterMaker(XOSView *xos);

  void makeMeters(void);

private:
  XOSView *_xos;
};

#endif
