//
//  Copyright (c) 1994, 1995, 2015, 2016, 2017, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef intmeter_h
#define intmeter_h

#include "cintmeter.h"
#include "intrstats.h"


class IntMeter : public ComIntMeter {
public:
    IntMeter(void);

protected:
    virtual std::map<size_t, uint64_t> getStats(void) override;

private:
    IntrStats _istats;
};

#endif
