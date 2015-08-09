//
//  Copyright (c) 1999, 2015
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//

#ifndef METERMAKER_H
#define METERMAKER_H

#include "rdb.h"



class Meter;
class XOSView;

// So we don't have to bring the .h
struct kstat_ctl;
typedef struct kstat_ctl kstat_ctl_t;



class MeterMaker {
public:
    MeterMaker(XOSView *xos);

    std::vector<Meter *> makeMeters(const ResDB &rdb);

private:
    XOSView *_xos;
    std::vector<Meter *> _meters;

    void cpuFactory(const ResDB &rdb, kstat_ctl_t *kc);
};


#endif
