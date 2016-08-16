//
//  Copyright (c) 2015, 2016
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
    MeterMaker(void);

    std::vector<Meter *> makeMeters(const ResDB &rdb);

private:
    void cpuFactory(const ResDB &rdb);
    void tzoneFactory(void);
};


#endif
