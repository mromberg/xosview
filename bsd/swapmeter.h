//  
//  Copyright (c) 1995, 1996, 1997-2002 by Brian Grayson (bgrayson@netbsd.org)
//
//  This file was written by Brian Grayson for the NetBSD and xosview
//    projects.
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//
// $Id: swapmeter.h,v 1.10 2002/03/22 03:23:41 bgrayson Exp $
//
#ifndef _SWAPMETER_H_
#define _SWAPMETER_H_

#define SWAPMETER_H_CVSID "$Id: swapmeter.h,v 1.10 2002/03/22 03:23:41 bgrayson Exp $"

#include "fieldmetergraph.h"


class SwapMeter : public FieldMeterGraph {
public:
  SwapMeter( XOSView *parent );
  ~SwapMeter( void );

  const char *name( void ) const { return "SwapMeter"; }  
  void checkevent( void );

  void checkResources( void );
protected:

  void getswapinfo( void );
private:
#ifdef HAVE_SWAPCTL
  int useSwapCtl;
#endif
};


#endif
