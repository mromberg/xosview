//  
//  Copyright (c) 1994, 1995 by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: MeterMaker.h,v 1.3 1996/11/24 04:35:07 bgrayson Exp $
//
#ifndef _MeterMaker_h
#define _MeterMaker_h

#define METERMAKER_H_CVSID "$Id: MeterMaker.h,v 1.3 1996/11/24 04:35:07 bgrayson Exp $"

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
