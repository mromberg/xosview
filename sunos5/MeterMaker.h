//  
// $Id: MeterMaker.h,v 1.1 1998/06/22 14:26:06 bgrayson Exp $
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
