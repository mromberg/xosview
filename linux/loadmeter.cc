//  
//  Copyright (c) 1994, 1995 by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  This file may be distributed under terms of the GPL
//
//  Most of this code was written by Werner Fink <werner@suse.de>.
//  Only small changes were made on my part (M.R.)
//
// $Id: loadmeter.cc,v 1.8 1999/11/06 22:48:17 romberg Exp $
//
#include "loadmeter.h"
#include "xosview.h"
#include <fstream.h>
#include <stdlib.h>

static const char LOADFILENAME[] = "/proc/loadavg";

LoadMeter::LoadMeter( XOSView *parent )
  : FieldMeterGraph( parent, 2, "LOAD", "PROCS/MIN", 1, 1, 0 ){
  lastalarmstate = -1;
  total_ = 2.0;
}

LoadMeter::~LoadMeter( void ){
}

void LoadMeter::checkResources( void ){
  FieldMeterGraph::checkResources();

  procloadcol_ = parent_->allocColor(parent_->getResource( "loadProcColor" ));
  warnloadcol_ = parent_->allocColor(parent_->getResource( "loadWarnColor" ));
  critloadcol_ = parent_->allocColor(parent_->getResource( "loadCritColor" ));

  setfieldcolor( 0, procloadcol_ );
  setfieldcolor( 1, parent_->getResource( "loadIdleColor" ) );
  priority_ = atoi (parent_->getResource( "loadPriority" ) );
  useGraph_ = parent_->isResourceTrue( "loadGraph" );
  dodecay_ = parent_->isResourceTrue( "loadDecay" );
  SetUsedFormat (parent_->getResource("loadUsedFormat"));

  warnThreshold = atoi (parent_->getResource("loadWarnThreshold"));
  critThreshold = atoi (parent_->getResource("loadCritThreshold"));


  if (dodecay_){
    //  Warning:  Since the loadmeter changes scale occasionally, old
    //  decay values need to be rescaled.  However, if they are rescaled,
    //  they could go off the edge of the screen.  Thus, for now, to
    //  prevent this whole problem, the load meter can not be a decay
    //  meter.  The load is a decaying average kind of thing anyway,
    //  so having a decaying load average is redundant.
    cerr << "Warning:  The loadmeter can not be configured as a decay\n"
         << "  meter.  See the source code (" << __FILE__ << ") for further\n"
         << "  details.\n";
    dodecay_ = 0;
  }
}

void LoadMeter::checkevent( void ){
  getloadinfo();
  drawfields();
}


void LoadMeter::getloadinfo( void ){
  ifstream loadinfo( LOADFILENAME );

  if ( !loadinfo ){
    cerr <<"Can not open file : " <<LOADFILENAME <<endl;
    parent_->done(1);
    return;
  }

  loadinfo >> fields_[0];

  if ( fields_[0] <  warnThreshold ) alarmstate = 0;
  else
  if ( fields_[0] >= critThreshold ) alarmstate = 2;
  else
  /* if fields_[0] >= warnThreshold */ alarmstate = 1;
  
  if ( alarmstate != lastalarmstate ){
    if ( alarmstate == 0 ) setfieldcolor( 0, procloadcol_ );
    else
    if ( alarmstate == 1 ) setfieldcolor( 0, warnloadcol_ );
    else
    /* if alarmstate == 2 */ setfieldcolor( 0, critloadcol_ );
    if (dolegends_) drawlegend();
    lastalarmstate = alarmstate;
  }
  
  if ( fields_[0]*5.0<total_ )
    total_ = fields_[0];
  else
  if ( fields_[0]>total_ )
    total_ = fields_[0]*5.0;
      
  if ( total_ < 1.0)
    total_ = 1.0;
    
  fields_[1] = (float) (total_ - fields_[0]);

  setUsed(fields_[0], (float) 1.0);
}
