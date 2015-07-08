//
//  Copyright (c) 1994, 1995, 2015 by Mike Romberg ( romberg@fsl.noaa.gov )
//  Copyright (c) 1995, 1996, 1997-2002 by Brian Grayson (bgrayson@netbsd.org)
//
//  Most of this code was written by Werner Fink <werner@suse.de>
//  Only small changes were made on my part (M.R.)
//  And the near-trivial port to NetBSD was by bgrayson.
//
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//
// $Id: loadmeter.h,v 1.10 2002/03/22 03:23:40 bgrayson Exp $
//
#ifndef _LOADMETER_H_
#define _LOADMETER_H_

#define LOADMETER_H_CVSID "$Id: loadmeter.h,v 1.10 2002/03/22 03:23:40 bgrayson Exp $"

#include "fieldmetergraph.h"


class LoadMeter : public FieldMeterGraph {
public:
  LoadMeter( XOSView *parent );
  ~LoadMeter( void );

  virtual std::string name( void ) const { return "LoadMeter"; }
  void checkevent( void );

  void checkResources( void );
protected:

  void getloadinfo( void );
  unsigned long procloadcol_, warnloadcol_, critloadcol_;
private:
  int warnThreshold, critThreshold, alarmstate, lastalarmstate;
};


#endif
