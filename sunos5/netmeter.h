//  
//  Copyright (c) 1994, 1995 by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: netmeter.h,v 1.1 1999/11/06 23:12:34 romberg Exp $
//
#ifndef _NETMETER_H_
#define _NETMETER_H_

#include "fieldmetergraph.h"
#include <kstat.h>
#define NNETS 100
#define GUESS_MTU 1500

class Host;

class NetMeter : public FieldMeterGraph {
public:
  NetMeter(XOSView *parent, kstat_ctl_t *_kc, float max);
  ~NetMeter( void );

  const char *name( void ) const { return "NetMeter"; }  
  void checkevent( void );

  void checkResources( void );
protected:
  float maxpackets_;

private:
  long long _lastBytesIn, _lastBytesOut;

  void adjust(void);
  int nnet;
  kstat_ctl_t *kc;
  kstat_t *nnets[NNETS];
  int packetsize[NNETS];
};

#endif
