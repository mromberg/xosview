//  
// $Id: MeterMaker.h,v 1.2 1998/06/22 15:57:28 bgrayson Exp $
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
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
