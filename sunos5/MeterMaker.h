//
//  Copyright (c) 1999, 2015, 2016
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//

#ifndef METERMAKER_H
#define METERMAKER_H

#include "cmetermaker.h"

// So we don't have to bring the .h
class ResDB;
struct kstat_ctl;
typedef struct kstat_ctl kstat_ctl_t;



class MeterMaker : public ComMeterMaker {
public:
    MeterMaker(void);

    std::vector<Meter *> makeMeters(const ResDB &rdb);

private:
    void cpuFactory(const ResDB &rdb, kstat_ctl_t *kc);
};


#endif
