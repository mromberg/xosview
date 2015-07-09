//
//  Copyright (c) 1994, 1995, 2015 by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: intmeter.cc,v 1.6 2003/10/09 04:12:48 bgrayson Exp $
//
#include <stdlib.h>		/*  For atoi().  */
#include "intmeter.h"
#include "kernel.h"




IntMeter::IntMeter( XOSView *parent,
                    const char *, const char *, int dolegends,
                    int dousedlegends )
  //  For platforms like the Mac with less than 16 interrupts, we
  //  ought to auto-detect how many there are.  I'll delay adding the
  //  code to do this for a few weeks, so that we can get version 1.6
  //  out the door.
  //  Same goes for platforms (like Alpha) with more than 16
  //  interrupts.
  : BitMeter( parent, "INTS", "IRQs", 16 /*BSDNumInts()*/,
              dolegends, dousedlegends ) {
  for ( int i = 0 ; i < 16 ; i++ )
    irqs_[i] = lastirqs_[i] = 0;
}

IntMeter::~IntMeter( void ){
}

void IntMeter::checkevent( void ){
  getirqs();

  for ( int i = 0 ; i < 16 ; i++ ){
    bits_[i] = ((irqs_[i] - lastirqs_[i]) != 0);
    lastirqs_[i] = irqs_[i];
  }

  BitMeter::checkevent();
}

void IntMeter::checkResources( void ){
  if (!BSDIntrInit()) {
    disableMeter();
  }
  BitMeter::checkResources();
  if (!disabled_) {
      onColor_  = parent_->g().allocColor( parent_->getResource( "intOnColor" ) );
      offColor_ = parent_->g().allocColor( parent_->getResource( "intOffColor" ) );
      priority_ = util::stoi( parent_->getResource( "intPriority" ) );
  }
}

void
IntMeter::getirqs( void )
{
    BSDGetIntrStats (irqs_);
}
