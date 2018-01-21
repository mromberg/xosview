//
//  Copyright (c) 2013, 2015, 2018
//  by Tomi Tapper ( tomi.o.tapper@student.jyu.fi )
//
//  Based on linux/btrymeter.h:
//  Copyright (c) 1997, 2005, 2006 by Mike Romberg ( mike.romberg@noaa.gov )
//
//  This file may be distributed under terms of the GPL
//

#ifndef btrymeter_h
#define btrymeter_h

#include "fieldmeter.h"



class BtryMeter : public FieldMeter {
public:
    BtryMeter(void);
    ~BtryMeter(void);

    virtual std::string resName(void) const { return "battery"; }
    void checkevent(void);
    void checkResources(const ResDB &rdb);

private:
    unsigned long _leftColor, _usedColor, _chargeColor, _fullColor,
        _lowColor, _critColor, _noneColor;
    unsigned int  _oldState;

    void getstats(void);
};


#endif
