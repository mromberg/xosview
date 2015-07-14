//
//  Copyright (c) 1994, 1995, 2015 by Mike Romberg ( romberg@fsl.noaa.gov )
//  2007 by Samuel Thibault ( samuel.thibault@ens-lyon.org )
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: memmeter.h,v 1.1 2008/02/28 23:43:06 romberg Exp $
//
#ifndef _MEMMETER_H_
#define _MEMMETER_H_

#include "fieldmetergraph.h"

extern "C" {
#include <mach/vm_statistics.h>
}

class MemMeter : public FieldMeterGraph {
public:
  MemMeter( XOSView *parent );
  ~MemMeter( void );

  virtual std::string name( void ) const { return "MemMeter"; }
  void checkevent( void );

  void checkResources( void );
protected:

  void getmeminfo( void );
private:

  struct vm_statistics vmstats;
};


#endif
