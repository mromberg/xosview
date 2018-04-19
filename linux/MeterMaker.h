//
//  Copyright (c) 1994, 1995, 2006, 2015, 2016, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef MeterMaker_h
#define MeterMaker_h

#include "cmetermaker.h"

class ResDB;


class MeterMaker : public ComMeterMaker {
public:
    mlist makeMeters(const ResDB &rdb);

private:
    void cpuFactory(const ResDB &rdb, mlist &meters) const;
    void serialFactory(const ResDB &rdb, mlist &meters) const;
    void intFactory(const ResDB &rdb, mlist &meters) const;
    void tzoneFactory(mlist &meters) const;
    void lmsTempFactory(const ResDB &rdb, mlist &meters) const;
};


#endif
