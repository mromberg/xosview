//  
//  Copyright (c) 1999 by Brian Grayson (bgrayson@netbsd.org)
//
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//
// $Id: intratemeter.h,v 1.3 2003/10/09 04:20:06 bgrayson Exp $
//
#ifndef _IRQRATEMETER_H_
#define _IRQRATEMETER_H_

#define IRQRATEMETER_H_CVSID "$Id: intratemeter.h,v 1.3 2003/10/09 04:20:06 bgrayson Exp $"

#include "fieldmetergraph.h"
#include "kernel.h"

class IrqRateMeter : public FieldMeterGraph {
public:
  IrqRateMeter( XOSView *parent );
  ~IrqRateMeter( void );

  const char *name( void ) const { return "IrqRateMeter"; }  
  void checkevent( void );

  void checkResources( void );
protected:
  unsigned long irqs_[NUM_INTR], lastirqs_[NUM_INTR];
  unsigned long delta;

  void getinfo( void );
  unsigned long oncol_, idlecol_;
  bool kernelHasStats_;
};
#endif
