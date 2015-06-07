//
//  Copyright (c) 1994, 1995, 2006, 2015 by Mike Romberg ( mike.romberg@noaa.gov )
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: swapmeter.cc,v 1.13 2008/02/28 22:26:48 romberg Exp $
//

#include "swapmeter.h"
#include "xosview.h"
#include <fstream>
#include <sstream>
#include <stdlib.h>

#ifdef USESYSCALLS
#if defined(GNULIBC) || defined(__GLIBC__)
#include <sys/sysinfo.h>
#else
#include <syscall.h>
#include <linux/kernel.h>
#endif
#endif


static const char MEMFILENAME[] = "/proc/meminfo";


SwapMeter::SwapMeter( XOSView *parent )
: FieldMeterGraph( parent, 2, "SWAP", "USED/FREE" ){

}

SwapMeter::~SwapMeter( void ){
}

void SwapMeter::checkResources( void ){
  FieldMeterGraph::checkResources();

  setfieldcolor( 0, parent_->getResource( "swapUsedColor" ) );
  setfieldcolor( 1, parent_->getResource( "swapFreeColor" ) );
  priority_ = util::stoi (parent_->getResource( "swapPriority" ));
  dodecay_ = parent_->isResourceTrue( "swapDecay" );
  useGraph_ = parent_->isResourceTrue( "swapGraph" );
  setUsedFormat (parent_->getResource("swapUsedFormat"));
}

void SwapMeter::checkevent( void ){
  getswapinfo();
  drawfields();
}


#ifdef USESYSCALLS
void SwapMeter::getswapinfo( void ){
  struct sysinfo sinfo;
  int unit;

#if defined(GNULIBC) || defined(__GLIBC__)
  sysinfo(&sinfo);
#else
  syscall( SYS_sysinfo, &sinfo );
#endif

  unit = (sinfo.mem_unit ? sinfo.mem_unit : 1);

  total_ = sinfo.totalswap * unit;
  fields_[0] = (sinfo.totalswap - sinfo.freeswap) * unit;
  fields_[1] = sinfo.freeswap * unit;

  if ( total_ == 0 ){
    total_ = 1;
    fields_[0] = 0;
    fields_[1] = 1;
  }

  if (total_)
    setUsed (fields_[0], total_);

}
#else
void SwapMeter::getswapinfo( void ){
  std::ifstream meminfo( MEMFILENAME );
  if ( !meminfo ){
    std::cerr <<"Cannot open file : " <<MEMFILENAME << std::endl;
    exit( 1 );
  }

  total_ = fields_[0] = fields_[1] = 0;

  char buf[256];
  string ignore;

  // Get the info from the "standard" meminfo file.
  while (!meminfo.eof()){
    meminfo.getline(buf, 256);
    std::istringstream line(std::string(buf, 256));

    if(!strncmp("SwapTotal", buf, strlen("SwapTotal")))
        line >> ignore >> total_;

    if(!strncmp("SwapFree", buf, strlen("SwapFree")))
        line >> ignore >> fields_[1];
  }

  fields_[0] = total_ - fields_[1];

  if ( total_ == 0 ){
    total_ = 1;
    fields_[0] = 0;
    fields_[1] = 1;
  }

  if (total_)
    setUsed (fields_[0], total_);
}
#endif
