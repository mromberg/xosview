//
//  Copyright (c) 2015, 2018
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
};


#endif
