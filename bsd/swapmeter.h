//  
//  Copyright (c) 1995, 1996, 1997 by Brian Grayson (bgrayson@ece.utexas.edu)
//
//  This file was written by Brian Grayson for the NetBSD and xosview
//    projects.
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//
// $Id: swapmeter.h,v 1.7 1997/02/14 07:24:24 bgrayson Exp $
//
#ifndef _SWAPMETER_H_
#define _SWAPMETER_H_

#define SWAPMETER_H_CVSID "$Id: swapmeter.h,v 1.7 1997/02/14 07:24:24 bgrayson Exp $"

#include "fieldmeterdecay.h"


class SwapMeter : public FieldMeterDecay {
public:
  SwapMeter( XOSView *parent );
  ~SwapMeter( void );

  const char *name( void ) const { return "SwapMeter"; }  
  void checkevent( void );

  void checkResources( void );
protected:

  void getswapinfo( void );
private:
};


#endif
