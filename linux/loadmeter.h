//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
//  Most of this code was written by Werner Fink <werner@suse.de>
//  Only small changes were made on my part (M.R.)
//
#ifndef _LOADMETER_H_
#define _LOADMETER_H_


#include "fieldmetergraph.h"


class LoadMeter : public FieldMeterGraph {
public:
    LoadMeter( XOSView *parent );
    ~LoadMeter( void );

    std::string name( void ) const { return "LoadMeter"; }
    void checkevent( void );

    void checkResources( void );

protected:
    void getloadinfo( void );
    void getspeedinfo( void );

private:
    unsigned long _procloadcol, _warnloadcol, _critloadcol;

    int _warnThreshold, _critThreshold;
    enum AlarmState { NORM, WARN, CRIT };
    AlarmState _alarmstate, _lastalarmstate;
    size_t _old_cpu_speed, _cur_cpu_speed;
    bool _do_cpu_speed;
};

#endif
