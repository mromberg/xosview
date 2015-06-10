//
//  Copyright (c) 1994, 1995, 2006, 2008, 2015 by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
//  Most of this code was written by Werner Fink <werner@suse.de>.
//  Only small changes were made on my part (M.R.)
//
// $Id: loadmeter.cc,v 1.12 2008/02/29 00:38:18 romberg Exp $
//
#include "loadmeter.h"
#include "xosview.h"
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <math.h>

static const char LOADFILENAME[] = "/proc/loadavg";
static const char SPEEDFILENAME[] = "/proc/cpuinfo";


LoadMeter::LoadMeter( XOSView *parent )
  : FieldMeterGraph( parent, 2, "LOAD", "PROCS/MIN", 1, 1, 0 ){
  lastalarmstate = -1;
  total_ = 2.0;
  old_cpu_speed_= cur_cpu_speed_=0;
  do_cpu_speed = 0;

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
  priority_ = util::stoi (parent_->getResource( "loadPriority" ));
  useGraph_ = parent_->isResourceTrue( "loadGraph" );
  dodecay_ = parent_->isResourceTrue( "loadDecay" );
  setUsedFormat (parent_->getResource("loadUsedFormat"));

  warnThreshold = util::stoi (parent_->getResource("loadWarnThreshold"));
  critThreshold = util::stoi (parent_->getResource("loadCritThreshold"));

  do_cpu_speed  = parent_->isResourceTrue( "loadCpuSpeed" );

  if (dodecay_){
    //  Warning:  Since the loadmeter changes scale occasionally, old
    //  decay values need to be rescaled.  However, if they are rescaled,
    //  they could go off the edge of the screen.  Thus, for now, to
    //  prevent this whole problem, the load meter can not be a decay
    //  meter.  The load is a decaying average kind of thing anyway,
    //  so having a decaying load average is redundant.
    std::cerr << "Warning:  The loadmeter can not be configured as a decay\n"
         << "  meter.  See the source code (" << __FILE__ << ") for further\n"
         << "  details.\n";
    dodecay_ = 0;
  }
}

void LoadMeter::checkevent( void ){
  getloadinfo();
  if ( do_cpu_speed ) {
         getspeedinfo();
         if ( old_cpu_speed_ != cur_cpu_speed_ ) {
        // update the legend:
        std::ostringstream legnd;
        XOSDEBUG("SPEED: %d\n",cur_cpu_speed_);
        legnd << "PROCS/MIN" << " " << cur_cpu_speed_ << " MHz"<< std::ends;
            legend( legnd.str().c_str() );
        if (dolegends_) {
                        drawlegend();
                        parent_->reallydraw();
                }
     }
  }

  drawfields();
}


void LoadMeter::getloadinfo( void ){
  std::ifstream loadinfo( LOADFILENAME );

  if ( !loadinfo ){
    std::cerr <<"Can not open file : " <<LOADFILENAME << std::endl;
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
// just check /proc/cpuinfo for the speed of cpu0
// (be ignorant on multi-cpus being on different speeds)
// display is intended mainly for laptops ...
// (yes - i know about devices/system/cpu/cpu0/cpufreq )
void LoadMeter::getspeedinfo( void ){

  std::string filename;
  std::ifstream speedinfo;
  std::string inp_line;

  std::string argname;
  std::string argval;

  speedinfo.open(SPEEDFILENAME );
  while ( speedinfo.good() ) {
    argname.clear();
    std::getline(speedinfo,argname,':');
    argval.clear();
    std::getline(speedinfo,argval);
        // XOSDEBUG("speed: a=\"%s\" v=\"%s\"\n",argname.c_str(),argval.c_str() );

    if ( argname.substr(0,7) == "cpu MHz" ) {
        //XOSDEBUG("SPEED: %s\n",argval.c_str() );
        old_cpu_speed_ = cur_cpu_speed_;
        cur_cpu_speed_ = util::stoi(argval);
        // Make it a round number
        cur_cpu_speed_ = 100 * (int) nearbyint ( ((double) cur_cpu_speed_ )
          / 100 );
        break;
    }
  }
  speedinfo.close(); speedinfo.clear();

}
