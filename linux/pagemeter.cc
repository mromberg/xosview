//  
//  Copyright (c) 1996 by Massimiliano Ghilardi ( ghilardi@cibs.sns.it )
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: pagemeter.cc,v 1.1 1996/10/27 23:23:01 mromberg Exp $
//
#include "pagemeter.h"
#include "xosview.h"
#include <fstream.h>
#include <stdlib.h>


static const char STATFILENAME[] = "/proc/stat";


PageMeter::PageMeter( XOSView *parent, float max )
  : FieldMeterDecay( parent, 3, "PAGE", "IN/OUT/IDLE" ){
  for ( int i = 0 ; i < 2 ; i++ )
    for ( int j = 0 ; j < 2 ; j++ )
      pageinfo_[j][i] = 0;

  maxspeed_ = max;
  pageindex_ = 0;
}

PageMeter::~PageMeter( void ){
}

void PageMeter::checkResources( void ){
  FieldMeterDecay::checkResources();

  setfieldcolor( 0, parent_->getResource( "pageInColor" ) );
  setfieldcolor( 1, parent_->getResource( "pageOutColor" ) );
  setfieldcolor( 2, parent_->getResource( "pageIdleColor" ) );
  priority_ = atoi (parent_->getResource( "pagePriority" ) );
  maxspeed_ *= priority_ / 10.0;
  dodecay_ = !strcmp (parent_->getResource( "pageDecay" ), "True" );
}

void PageMeter::checkevent( void ){
  getpageinfo();
  drawfields();
}

void PageMeter::getpageinfo( void ){
  total_ = 0;
  char buf[256];
  ifstream stats( STATFILENAME );

  if ( !stats ){
    cerr <<"Cannot open file : " <<STATFILENAME <<endl;
    exit( 1 );
  }

  stats.getline( buf, 256 );
  stats.getline( buf, 256 );
  stats.getline( buf, 256 );

  stats >>buf >>pageinfo_[pageindex_][0]
	      >>pageinfo_[pageindex_][1];

  int oldindex = (pageindex_+1)%2;
  
  for ( int i = 0; i < 2; i++ ) {
    if ( pageinfo_[oldindex][i] == 0 )
      pageinfo_[oldindex][i] = pageinfo_[pageindex_][i];

    fields_[i] = pageinfo_[pageindex_][i] - pageinfo_[oldindex][i];
    total_ += fields_[i];
  }

  if ( total_ > maxspeed_ )
    fields_[2] = 0.0;
  else {
    fields_[2] = maxspeed_ - total_;
    total_ = maxspeed_;
  }

  used( (int)((100 * (total_ - fields_[2])) / maxspeed_) );
  pageindex_ = (pageindex_ + 1) % 2;
}
