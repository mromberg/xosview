//
//  Copyright (c) 1999, 2015
//  by Mike Romberg (mike-romberg@comcast.net)
//
//  This file may be distributed under terms of the GPL
//
#ifndef DISKMETER_H
#define DISKMETER_H

#include "fieldmetergraph.h"

// To keep the header in the .cc file
struct kstat_ctl;
typedef kstat_ctl kstat_ctl_t;
struct kstat;
typedef kstat kstat_t;



class DiskMeter : public FieldMeterGraph {
public:
    DiskMeter( XOSView *parent, kstat_ctl_t *_kc, float max );
    ~DiskMeter( void );

    virtual std::string name( void ) const { return "DiskMeter"; }
    void checkevent( void );

    void checkResources( void );

protected:
    void getdiskinfo( void );

private:
    u_longlong_t read_prev_;
    u_longlong_t write_prev_;
    float maxspeed_;
    kstat_ctl_t *kc;
    std::vector<kstat_t *>part;
    size_t _npart;
};

#endif
