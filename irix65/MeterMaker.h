//  
// $Id: MeterMaker.h,v 1.1 2001/10/10 15:58:22 eile Exp $
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

    int setupSadc();
};

#endif
