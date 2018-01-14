//
//  Copyright (c) 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef BTRYMETER_H
#define BTRYMETER_H

#include "fieldmeter.h"

#include <string>


class BtryMeter : public FieldMeter {
public:
    BtryMeter( void );
    ~BtryMeter( void );

    std::string resName( void ) const { return "battery"; }
    void checkevent( void );

    void checkResources(const ResDB &rdb);

private:
    unsigned long _critColor, _lowColor, _leftColor, _chargeColor;
    unsigned long _fullColor, _noneColor;
};


#endif
