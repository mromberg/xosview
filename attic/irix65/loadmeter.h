//
//  Copyright (c) 1994, 1995, 2004, 2006, 2015
//  Initial port performed by Stefan Eilemann (eilemann@gmail.com)
//
//  This file may be distributed under terms of the GPL
//
#ifndef LOADMETER_H
#define LOADMETER_H

#include "fieldmetergraph.h"

#include <rpcsvc/rstat.h>

class LoadMeter : public FieldMeterGraph {
public:
    LoadMeter(XOSView *parent);
    ~LoadMeter(void);

    virtual std::string name(void) const { return "LoadMeter"; }
    void checkevent(void);

    void checkResources(void);

protected:
    void getloadinfo(void);

    unsigned long procloadcol_, warnloadcol_, critloadcol_;

private:
    int warnThreshold, critThreshold, alarmstate, lastalarmstate;
    char hostname[256];
    struct statstime res;
};

#endif
