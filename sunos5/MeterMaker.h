//
//  Copyright (c) 1999, 2015
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//

#ifndef METERMAKER_H
#define METERMAKER_H

#include "rdb.h"

#include <vector>

class Meter;

// So we don't have to bring the .h
struct kstat_ctl;
typedef struct kstat_ctl kstat_ctl_t;



class MeterMaker {
public:
    MeterMaker(void);

    std::vector<Meter *> makeMeters(const ResDB &rdb);

private:
    std::vector<Meter *> _meters;

    void cpuFactory(const ResDB &rdb, kstat_ctl_t *kc);
};


#endif
