//  
//  Copyright (c) 1994, 1995 by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: netmeter.h,v 1.3 1998/05/16 20:31:54 mromberg Exp $
//
#ifndef _NETMETER_H_
#define _NETMETER_H_

#include "fieldmeterdecay.h"
#include "timer.h"

class Host;

class NetMeter : public FieldMeterDecay {
public:
  NetMeter(XOSView *parent, float max);
  ~NetMeter( void );

  const char *name( void ) const { return "NetMeter"; }  
  void checkevent( void );

  void checkResources( void );
protected:
  float maxpackets_;

private:
  int _ipsock;
  Timer _timer;
  unsigned long _lastBytesIn, _lastBytesOut;
  char *_netfilename;
  int _bytesInDev;

  void adjust(void);
  void checkOSVersion(void);

  void checkeventOld(void);
  void checkeventNew(void);
};

#endif
