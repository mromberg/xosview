//
//  Copyright (c) 1994, 1995, 2006, 2008, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
//  Most of this code was written by Werner Fink <werner@suse.de>.
//  Only small changes were made on my part (M.R.)
//

#include "cloadmeter.h"

#include <sstream>
#include <iomanip>




ComLoadMeter::ComLoadMeter( XOSView *parent )
    : FieldMeterGraph( parent, 2, "LOAD", "PROCS/MIN" ),
      _alarmstate(NORM), _lastalarmstate(NORM), _do_cpu_speed(false) {
}


ComLoadMeter::~ComLoadMeter( void ){
}


void ComLoadMeter::checkResources(const ResDB &rdb){
    FieldMeterGraph::checkResources(rdb);

    _procloadcol = rdb.getColor("loadProcColor" );
    _warnloadcol = rdb.getColor("loadWarnColor" );
    _critloadcol = rdb.getColor("loadCritColor" );

    setfieldcolor( 0, _procloadcol );
    setfieldcolor( 1, rdb.getColor( "loadIdleColor" ) );

    _warnThreshold = util::stoi (rdb.getResource("loadWarnThreshold"));
    _critThreshold = util::stoi (rdb.getResource("loadCritThreshold"));

    // The max displayed on the bar will be the crit
    // legend still displays total load.
    total_ = _critThreshold;

    _do_cpu_speed  = rdb.isResourceTrue( "loadCpuSpeed" );
}


void ComLoadMeter::checkevent( void ){
    setLoadInfo(getLoad());

    if ( _do_cpu_speed ) {
        uint64_t speed = getCPUSpeed();
        if (speed) {
            // update the legend:
            std::ostringstream legnd;
            logDebug << "SPEED: " << speed << std::endl;
            legnd << "PROCS/MIN" << " "
                  << std::setfill(' ') << std::setw(5) << speed / 1000000
                  << " MHz";
            legend( legnd.str() );
        }
    }
}


void ComLoadMeter::setLoadInfo(float load){
    fields_[0] = load;

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

        _lastalarmstate = _alarmstate;
    }

    fields_[1] = (float) (total_ - fields_[0]);
    if (fields_[1] < 0) // peaked the meter
        fields_[1] = 0.0;
    logDebug << "loadMeter: " << fields_[0] << ", " << fields_[1] << ", "
             << total_ << std::endl;
    setUsed(fields_[0], (float) 1.0);
}
