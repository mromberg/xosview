//  
// $Id: MeterMaker.h,v 1.3 2002/02/20 11:15:04 eile Exp $
//  Initial port performed by Stefan Eilemann (eile@sgi.com)
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
