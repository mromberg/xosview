//  
// $Id: MeterMaker.h,v 1.2 2002/02/19 07:45:10 eile Exp $
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

#ifdef USE_SAR
    int setupSar();
#endif
};

#endif
