//
//  Copyright (c) 2014, 2015, 2018 by Tomi Tapper <tomi.o.tapper@jyu.fi>
//
//  File based on bsd/intratemeter.* by
//  Copyright (c) 1999 by Brian Grayson (bgrayson@netbsd.org)
//
//  This file may be distributed under terms of the GPL
//
//

#ifndef intratemeter_h
#define intratemeter_h

#include "cintratemeter.h"
#include "kstats.h"



class IrqRateMeter : public ComIrqRateMeter {
public:
    IrqRateMeter(kstat_ctl_t *kc);

protected:
    virtual float getIrqRate(void) override;

private:
    uint64_t _lastirqcount;
    kstat_ctl_t *_kc;
    KStatList *_cpus;

    KStatList &cpus(void) const { return *_cpus; }
};


#endif
