//
//  Copyright (c) 1994, 1995, 2006, 2015 by Mike Romberg ( mike.romberg@noaa.gov )
//
//  This file may be distributed under terms of the GPL
//
//  Most of this code was written by Werner Fink <werner@suse.de>
//  Only small changes were made on my part (M.R.)
//
// $Id: loadmeter.h,v 1.7 2006/02/18 04:33:06 romberg Exp $
//
#ifndef _LOADMETER_H_
#define _LOADMETER_H_


#include "fieldmetergraph.h"


class LoadMeter : public FieldMeterGraph {
public:
  LoadMeter( XOSView *parent );
  ~LoadMeter( void );

  std::string name( void ) const { return "LoadMeter"; }
  void checkevent( void );

  void checkResources( void );
protected:

  void getloadinfo( void );
  unsigned long procloadcol_, warnloadcol_, critloadcol_;
  void getspeedinfo( void );

private:
   int warnThreshold, critThreshold, alarmstate, lastalarmstate;
   int old_cpu_speed_, cur_cpu_speed_;
   int do_cpu_speed;
};


#endif
