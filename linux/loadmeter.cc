//
//  Copyright (c) 1994, 1995, 2006, 2008, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
//  Most of this code was written by Werner Fink <werner@suse.de>.
//  Only small changes were made on my part (M.R.)
//

#include "loadmeter.h"
#include "xosview.h"

#include <fstream>
#include <sstream>
#include <cmath>
#include <iomanip>


static const char LOADFILENAME[] = "/proc/loadavg";
static const char SPEEDFILENAME[] = "/proc/cpuinfo";


LoadMeter::LoadMeter( XOSView *parent )
    : FieldMeterGraph( parent, 2, "LOAD", "PROCS/MIN", 1, 1, 0 ),
      _alarmstate(NORM), _lastalarmstate(NORM), _old_cpu_speed(0),
      _cur_cpu_speed(0), _do_cpu_speed(false) {
    // reasonable default state.
    for (size_t i = 0 ; i < fields_.size() ; i++)
        fields_[i] = 0.0;
    total_ = 2.0;
}

LoadMeter::~LoadMeter( void ){
}

void LoadMeter::checkResources( void ){
    FieldMeterGraph::checkResources();

    _procloadcol = parent_->g().allocColor(parent_->getResource(
          "loadProcColor" ));
    _warnloadcol = parent_->g().allocColor(parent_->getResource(
          "loadWarnColor" ));
    _critloadcol = parent_->g().allocColor(parent_->getResource(
          "loadCritColor" ));

    setfieldcolor( 0, _procloadcol );
    setfieldcolor( 1, parent_->getResource( "loadIdleColor" ) );
    priority_ = util::stoi (parent_->getResource( "loadPriority" ));
    useGraph_ = parent_->isResourceTrue( "loadGraph" );
    dodecay_ = parent_->isResourceTrue( "loadDecay" );
    setUsedFormat (parent_->getResource("loadUsedFormat"));

    _warnThreshold = util::stoi (parent_->getResource("loadWarnThreshold"));
    _critThreshold = util::stoi (parent_->getResource("loadCritThreshold"));

    // The max displayed on the bar will be the crit
    // legend still displays total load.
    total_ = _critThreshold;

    _do_cpu_speed  = parent_->isResourceTrue( "loadCpuSpeed" );
}

void LoadMeter::checkevent( void ){
    getloadinfo();
    if ( _do_cpu_speed ) {
        getspeedinfo();
        if ( _old_cpu_speed != _cur_cpu_speed ) {
            // update the legend:
            std::ostringstream legnd;
            logDebug << "SPEED: " << _cur_cpu_speed << std::endl;
            legnd << "PROCS/MIN" << " "
                  << std::setfill(' ') << std::setw(5) << _cur_cpu_speed
                  << " MHz"<< std::ends;
            legend( legnd.str().c_str() );
            if (dolegends()) {
                drawlegend(parent_->g());
                //parent_->reallydraw();
            }
        }
    }

    drawfields(parent_->g());
}


void LoadMeter::getloadinfo( void ){
    std::ifstream loadinfo( LOADFILENAME );

    if ( !loadinfo )
        logFatal << "Can not open file : " << LOADFILENAME << std::endl;

    loadinfo >> fields_[0];

    if ( fields_[0] <  _warnThreshold )
        _alarmstate = NORM;
    else
        if ( fields_[0] >= _critThreshold )
            _alarmstate = CRIT;
        else
            /* if fields_[0] >= warnThreshold */
            _alarmstate = WARN;

    if ( _alarmstate != _lastalarmstate ){
        switch (_alarmstate) {
        case NORM:
            setfieldcolor( 0, _procloadcol );
            break;
        case WARN:
            setfieldcolor( 0, _warnloadcol );
            break;
        case CRIT:
            setfieldcolor( 0, _critloadcol );
            break;
        default:
            logBug << "Unknown alarm state: " << _alarmstate << std::endl;
        };

        if (dolegends())
            drawlegend(parent_->g());
        _lastalarmstate = _alarmstate;
    }

    fields_[1] = (float) (total_ - fields_[0]);
    if (fields_[1] < 0) // peaked the meter
        fields_[1] = 0.0;
    logDebug << "loadMeter: " << fields_[0] << ", " << fields_[1] << ", "
             << total_ << std::endl;
    setUsed(fields_[0], (float) 1.0);
}


// just check /proc/cpuinfo for the speed of cpu0
// (be ignorant on multi-cpus being on different speeds)
// display is intended mainly for laptops ...
// (yes - i know about devices/system/cpu/cpu0/cpufreq )
void LoadMeter::getspeedinfo( void ){

    std::string filename;
    std::string inp_line;

    std::string argname;
    std::string argval;

    std::ifstream speedinfo(SPEEDFILENAME);
    while ( speedinfo.good() ) {
        argname.clear();
        std::getline(speedinfo,argname,':');
        argval.clear();
        std::getline(speedinfo,argval);
        // XOSDEBUG("speed: a=\"%s\" v=\"%s\"\n",
        //     argname.c_str(),argval.c_str() );

        if ( argname.substr(0,7) == "cpu MHz" ) {
            //XOSDEBUG("SPEED: %s\n",argval.c_str() );
            _old_cpu_speed = _cur_cpu_speed;
            _cur_cpu_speed = util::stoi(argval);
            // Make it a round number
            _cur_cpu_speed = 100 * (int) nearbyint ( ((double) _cur_cpu_speed )
              / 100 );
            break;
        }
    }
}
