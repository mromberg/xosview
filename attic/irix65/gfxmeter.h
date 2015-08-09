//
//  Copyright (c) 1994, 1995, 2004, 2006, 2015
//  Initial port performed by Stefan Eilemann (eilemann@gmail.com)
//
//  This file may be distributed under terms of the GPL
//
#ifndef GFXMETER_H
#define GFXMETER_H

#include "sarmeter.h"

#include <rpcsvc/rstat.h>

class GfxMeter : public FieldMeterGraph {
public:
    GfxMeter(XOSView *parent, int max);
    ~GfxMeter(void);

    virtual std::string name(void) const { return "GfxMeter"; }
    void checkevent(void);

    void checkResources(void);

    static int nPipes( void );

protected:
    void getgfxinfo(void);

    unsigned long swapgfxcol_, warngfxcol_, critgfxcol_;

private:
    int warnThreshold, critThreshold, alarmstate, lastalarmstate;
    int _nPipes;
};

#endif
