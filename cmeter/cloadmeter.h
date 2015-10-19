//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
//  Most of this code was written by Werner Fink <werner@suse.de>
//  Only small changes were made on my part (M.R.)
//
#ifndef CLOADMETER_H
#define CLOADMETER_H


#include "fieldmetergraph.h"

#include <stdint.h>



class ComLoadMeter : public FieldMeterGraph {
public:
    ComLoadMeter( void );
    ~ComLoadMeter( void );

    std::string resName( void ) const { return "load"; }
    void checkevent( void );

    void checkResources(const ResDB &rdb);

protected:
    virtual float getLoad(void) = 0;
    virtual uint64_t getCPUSpeed(void) { return 0; } // cycles/sec

private:
    unsigned long _procloadcol, _warnloadcol, _critloadcol;

    int _warnThreshold, _critThreshold;
    enum AlarmState { NORM, WARN, CRIT };
    AlarmState _alarmstate, _lastalarmstate;
    bool _do_cpu_speed;

    void setLoadInfo(float load);
};


#endif
