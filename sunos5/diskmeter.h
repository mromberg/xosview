//
//  Copyright (c) 1999, 2015, 2018
//  by Mike Romberg (romberg@fsl.noaa.gov)
//
//  This file may be distributed under terms of the GPL
//

#ifndef diskmeter_h
#define diskmeter_h

#include "cdiskmeter.h"
#include "kstats.h"



class DiskMeter : public ComDiskMeter {
public:
    DiskMeter(kstat_ctl_t *kc);

protected:
    virtual std::pair<double, double> getRate(void) override;

private:
    uint64_t _read_prev, _write_prev;
    kstat_ctl_t *_kc;
    KStatList *_disks;

    KStatList &disks(void) const { return *_disks; }
};


#endif
