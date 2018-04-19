//
//  Copyright (c) 1994, 1995, 2006, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
//  Most of this code was written by Werner Fink <werner@suse.de>
//  Only small changes were made on my part (M.R.)
//
#ifndef cloadmeter_h
#define cloadmeter_h


#include "fieldmetergraph.h"



class ComLoadMeter : public FieldMeterGraph {
public:
    ComLoadMeter(void);

    virtual std::string resName(void) const override { return "load"; }
    virtual void checkevent(void) override;

    virtual void checkResources(const ResDB &rdb) override;

protected:
    virtual float getLoad(void) = 0;
    virtual uint64_t getCPUSpeed(void) { return 0; } // cycles/sec

private:
    enum AlarmState { NORM, WARN, CRIT };

    unsigned long _procloadcol;
    unsigned long _warnloadcol;
    unsigned long _critloadcol;
    int _warnThreshold;
    int _critThreshold;
    AlarmState _alarmstate, _lastalarmstate;
    bool _doCPUSpeed;

    void setLoadInfo(float load);
};


#endif
