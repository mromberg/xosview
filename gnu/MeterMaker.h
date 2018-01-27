//
//  Copyright (c) 1994, 1995, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//  2007 by Samuel Thibault ( samuel.thibault@ens-lyon.org )
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
};

#endif
