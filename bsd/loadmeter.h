//  
//  Copyright (c) 1994, 1995 by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  This file may be distributed under terms of the GPL
//
//  Most of this code was written by Werner Fink <werner@suse.de>
//  Only small changes were made on my part (M.R.)
//
// $Id: loadmeter.h,v 1.5 1997/02/14 05:41:57 bgrayson Exp $
//
#ifndef _LOADMETER_H_
#define _LOADMETER_H_

#define LOADMETER_H_CVSID "$Id: loadmeter.h,v 1.5 1997/02/14 05:41:57 bgrayson Exp $"

#include "fieldmeterdecay.h"


class LoadMeter : public FieldMeterDecay {
public:
  LoadMeter( XOSView *parent );
  ~LoadMeter( void );

  const char *name( void ) const { return "LoadMeter"; }  
  void checkevent( void );

  void checkResources( void );
protected:

  void getloadinfo( void );
  unsigned long procloadcol_, warnloadcol_;
private:
  int alarmThreshold;
};


#endif
