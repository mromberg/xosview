//
//  Copyright (c) 1994, 1995 by Mike Romberg ( romberg@fsl.noaa.gov )
//  2007 by Samuel Thibault ( samuel.thibault@ens-lyon.org )
//
//  This file may be distributed under terms of the GPL
//
// $Id: memmeter.cc,v 1.1 2008/02/28 23:43:06 romberg Exp $
//
#include "memmeter.h"
#include "xosview.h"
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <error.h>

extern "C" {
#include <mach/mach_traps.h>
#include <mach/mach_interface.h>
}

MemMeter::MemMeter( XOSView *parent )
: FieldMeterGraph( parent, 4, "MEM", "ACT/INACT/WIRE/FREE" ){
}

MemMeter::~MemMeter( void ){
}

void MemMeter::checkResources( void ){
  FieldMeterGraph::checkResources();

  setfieldcolor( 0, parent_->getResource( "memActiveColor" ) );
  setfieldcolor( 1, parent_->getResource( "memInactiveColor" ) );
  setfieldcolor( 2, parent_->getResource( "memCacheColor" ) );
  setfieldcolor( 3, parent_->getResource( "memFreeColor" ) );
  priority_ = atoi (parent_->getResource( "memPriority" ) );
  dodecay_ = parent_->isResourceTrue( "memDecay" );
  useGraph_ = parent_->isResourceTrue( "memGraph" );
  SetUsedFormat (parent_->getResource("memUsedFormat"));
}

void MemMeter::checkevent( void ){
  getmeminfo();
  drawfields();
}

void MemMeter::getmeminfo( void ){
  kern_return_t err;

  err = vm_statistics (mach_task_self(), &vmstats);
  if (err) {
    error (0, err, "vm_statistics");
    parent_->done(1);
    return;
  }

  fields_[0] = vmstats.active_count * vmstats.pagesize;
  fields_[1] = vmstats.inactive_count * vmstats.pagesize;;
  fields_[2] = vmstats.wire_count * vmstats.pagesize;;
  fields_[3] = vmstats.free_count * vmstats.pagesize;;
  total_ = fields_[0] + fields_[1] + fields_[2] + fields_[3];

  FieldMeterDecay::setUsed (total_ - fields_[3], total_);
}
