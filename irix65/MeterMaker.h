//  
// $Id: MeterMaker.h,v 1.4 2006/10/11 07:30:53 eile Exp $
//  Initial port performed by Stefan Eilemann (eilemann@gmail.com)
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
