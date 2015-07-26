//
//  Copyright (c) 1999, 2015
//  by Mike Romberg (romberg@fsl.noaa.gov)
//
//  This file may be distributed under terms of the GPL
//

#ifndef DISKMETER_H
#define DISKMETER_H

#include "fieldmetergraph.h"
#include "kstats.h"



class DiskMeter : public FieldMeterGraph {
public:
    DiskMeter( XOSView *parent, kstat_ctl_t *kc, float max );
    ~DiskMeter( void );

    virtual std::string name( void ) const { return "DiskMeter"; }
    void checkevent( void );
    void checkResources( void );

protected:
    void getdiskinfo( void );

private:
    uint64_t _read_prev, _write_prev;
    float _maxspeed;
    kstat_ctl_t *_kc;
    KStatList *_disks;
};


#endif
