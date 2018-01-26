//
//  Copyright (c) 1994, 1995, 2006, 2008, 2015, 2016, 2018
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




ComLoadMeter::ComLoadMeter( void )
    : FieldMeterGraph( 2, "LOAD", "PROCS/MIN" ),
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
    setLegendColor( 1, _procloadcol ); // so cpu speed uses same color.

    _warnThreshold = std::stoi (rdb.getResource("loadWarnThreshold"));
    _critThreshold = std::stoi (rdb.getResource("loadCritThreshold"));

    // The max displayed on the bar will be the crit
    // legend still displays total load.
    _total = _critThreshold;

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

    if ( load <  _warnThreshold )
        _alarmstate = NORM;
    else
        if ( load >= _critThreshold )
            _alarmstate = CRIT;
        else // load >= warnThreshold
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

    // _total set to _critThreshold in ctor
    _fields[0] = std::min(load, _total);
    _fields[1] = _total - _fields[0];
    if (_fields[1] < 0) // peaked the meter
        _fields[1] = 0.0;

    logDebug << "loadMeter: " << _fields[0] << ", " << _fields[1] << ", "
             << _total << std::endl;

    setUsed(load, 1.0);
}
