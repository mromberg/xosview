//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#ifndef METERMAKER_H
#define METERMAKER_H

#include "cmetermaker.h"

class ResDB;



class MeterMaker : public ComMeterMaker {
public:
    mlist makeMeters(const ResDB &rdb);
};


#endif
