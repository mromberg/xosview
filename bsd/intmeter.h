//
//  Copyright (c) 1994, 1995, 2015, 2016, 2017
//  by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  This file may be distributed under terms of the GPL
//

#ifndef INTMETER_H
#define INTMETER_H

#include "cintmeter.h"
#include "intrstats.h"


class IntMeter : public ComIntMeter {
public:
    IntMeter(void);

protected:
    virtual std::map<size_t, uint64_t> getStats(void) const;

private:
    mutable IntrStats _istats;
};


#endif
