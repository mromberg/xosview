//
//  Copyright (c) 1999, 2015
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//

#ifndef METERMAKER_H
#define METERMAKER_H

#include "pllist.h"


class Meter;
class XOSView;

// So we don't have to bring the .h
struct kstat_ctl;
typedef struct kstat_ctl kstat_ctl_t;



class MeterMaker : public PLList<Meter *> {
public:
    MeterMaker(XOSView *xos);

    void makeMeters(void);

private:
    XOSView *_xos;

    void cpuFactory(kstat_ctl_t *kc);
};


#endif
