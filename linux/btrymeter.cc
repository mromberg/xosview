//  
//  Copyright (c) 1997 by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: btrymeter.cc,v 1.3 1997/12/02 20:03:41 bgrayson Exp $
//
#include "btrymeter.h"
#include "xosview.h"
#include <fstream.h>
#include <stdlib.h>

static const char APMFILENAME[] = "/proc/apm";

BtryMeter::BtryMeter( XOSView *parent )
  : FieldMeter( parent, 2, "BTRY", "AVAIL/USED", 1, 0 ){
}

BtryMeter::~BtryMeter( void ){
}

void BtryMeter::checkResources( void ){
  FieldMeter::checkResources();

  setfieldcolor( 0, parent_->getResource( "batteryUsedColor" ) );
  setfieldcolor( 1, parent_->getResource( "batteryLeftColor" ) );

  priority_ = atoi (parent_->getResource( "batteryPriority" ) );
  SetUsedFormat(parent_->getResource( "batteryUsedFormat" ) );
}

void BtryMeter::checkevent( void ){
  getpwrinfo();

  drawfields();
}


void BtryMeter::getpwrinfo( void ){
  ifstream loadinfo( APMFILENAME );

  if ( !loadinfo ){
    cerr <<"Can not open file : " <<APMFILENAME <<endl;
    parent_->done(1);
    return;
  }

  char buff[256];
  loadinfo >> buff >> buff >> buff >> buff >> buff >> buff >> fields_[0];

  total_ = 100;

  fields_[1] = total_ - fields_[0];

  setUsed (fields_[0], total_);
}
