//  
//  Copyright (c) 1994, 1995 by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: memmeter.cc,v 1.3 1997/04/28 21:39:41 mromberg Exp $
//
#include "memmeter.h"
#include "xosview.h"
#include <unistd.h>
#include <stdlib.h>

MemMeter::MemMeter( XOSView *parent )
: FieldMeterDecay( parent, 5, "MEM", "TEXT/USED/BSTOR/OTHER/FREE" ){
  struct pst_static pststatic;

  pstat_getstatic( &pststatic, sizeof( struct pst_static ), 1, 0 );
  total_ = pststatic.physical_memory;

  stats_ = new struct pst_status[pststatic.max_proc];
}

void MemMeter::checkResources( void ){
  FieldMeterDecay::checkResources();

  setfieldcolor( 0, parent_->getResource( "memTextColor" ) );
  setfieldcolor( 1, parent_->getResource( "memUsedColor" ) );
  setfieldcolor( 2, parent_->getResource( "memCacheColor" ) );
  setfieldcolor( 3, parent_->getResource( "memOtherColor" ) );
  setfieldcolor( 4, parent_->getResource( "memFreeColor" ) );
  priority_ = atoi (parent_->getResource( "memPriority" ) );
  dodecay_ = !strcmp (parent_->getResource( "memDecay" ), "True" );
}

MemMeter::~MemMeter( void ){
  delete[] stats_;
}

void MemMeter::checkevent( void ){
  static int pass = 0;

  pass = (pass + 1)%5;
  if ( pass != 0 )
    return;
  
  getmeminfo();
  drawfields();
}

void MemMeter::getmeminfo( void ){
  struct pst_dynamic stats;

  pstat_getdynamic(&stats, sizeof( pst_dynamic ), 1, 0);

  struct pst_vminfo vmstats;
  pstat_getvminfo(&vmstats, sizeof(vmstats), 1, 0);

  fields_[0] = stats.psd_rmtxt + stats.psd_arm;
  fields_[1] = stats.psd_rm - stats.psd_rmtxt;
  fields_[2] = vmstats.psv_swapmem_cnt; // + vmstats.psv_swapper_mem;
  fields_[3] = total_ - fields_[0] - fields_[1] - fields_[2] - stats.psd_free;
  fields_[4] = stats.psd_free;

  FieldMeterDecay::used( (int)((100 * (total_ - fields_[3])) / total_) );
}


