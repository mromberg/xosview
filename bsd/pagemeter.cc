//  
//  Copyright (c) 1994, 1995 by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  NetBSD port:  
//  Copyright (c) 1995, 1996, 1997 by Brian Grayson (bgrayson@ece.utexas.edu)
//
//  This file was originally written by Brian Grayson for the NetBSD and
//    xosview projects.
//
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//
// $Id: pagemeter.cc,v 1.11 1998/09/18 18:18:11 bgrayson Exp $
//
#include <stdlib.h>		//  For atoi().  BCG
#include "general.h"
#include "pagemeter.h"
#include "kernel.h"		//  For NetBSD Page functions.

CVSID("$Id: pagemeter.cc,v 1.11 1998/09/18 18:18:11 bgrayson Exp $");
CVSID_DOT_H(PAGEMETER_H_CVSID);

PageMeter::PageMeter( XOSView *parent, double total )
: FieldMeterDecay( parent, 3, "PAGE", "IN/OUT/IDLE" ){
  total_ = total;
  BSDPageInit();
#ifdef UVM
  BSDGetUVMPageStats(&prev_);
#else
  BSDGetPageStats(&prev_);
#endif
}

PageMeter::~PageMeter( void ){ }

void PageMeter::checkResources( void ){
  FieldMeter::checkResources();

  //  The Active and Inactive colors are new.
  setfieldcolor( 0, parent_->getResource("pageInColor") );
  setfieldcolor( 1, parent_->getResource("pageOutColor") );
  setfieldcolor( 2, parent_->getResource("pageIdleColor") );
  priority_ = atoi (parent_->getResource("pagePriority"));
  dodecay_ = !strncasecmp (parent_->getResource("pageDecay"),"True", 5);
  SetUsedFormat (parent_->getResource("pageUsedFormat"));
}

void PageMeter::checkevent( void ){
  getpageinfo();
  drawfields();
}

void PageMeter::getpageinfo (void) {
//  Begin NetBSD-specific code...
#if defined(UVM)
  struct uvmexp uvm;
  BSDGetUVMPageStats(&uvm);
#else
  struct vmmeter vm;
  BSDGetPageStats(&vm);
#endif
#ifdef XOSVIEW_FREEBSD
  /* It depends, of course on what you want to measure.  I think, howver,
     that you want the sum of pages paged to swap (i.e. dirty pages) and
     pages paged to vnodes (i.e. mmap-ed files). (pavel 21-Jan-1998) */
  fields_[0] = vm.v_vnodepgsin - prev_.v_vnodepgsin +
      				vm.v_swappgsin - prev_.v_swappgsin;
  fields_[1] = vm.v_vnodepgsout - prev_.v_vnodepgsout +
      				vm.v_swappgsout - prev_.v_swappgsout;
  prev_ = vm;
#else
# if defined(UVM)
  fields_[0] = uvm.pageins - prev_.pageins;
  fields_[1] = uvm.pgswapout - prev_.pgswapout;
  prev_ = uvm;
# else
  fields_[0] = vm.v_pgpgin - prev_.v_pgpgin;
  fields_[1] = vm.v_pgpgout - prev_.v_pgpgout;
  prev_ = vm;
# endif
#endif
//  End NetBSD-specific code...
  if (total_ < fields_[0] + fields_[1])
    total_ = fields_[0] + fields_[1];

  fields_[2] = total_ - fields_[0] - fields_[1];
  setUsed (total_ - fields_[2], total_);
}
