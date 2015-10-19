//
//  Copyright (c) 1999, 2015
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//
#ifndef MEMMETER_H
#define MEMMETER_H

#include "fieldmetergraph.h"
#include "kstat.h"


class MemMeter : public FieldMeterGraph {
public:
    MemMeter(kstat_ctl_t *kcp);
    ~MemMeter(void);

    virtual std::string resName(void) const { return "mem"; }
    void checkevent( void );
    void checkResources(const ResDB &rdb);

private:
    int pageSize;
    kstat_ctl_t *kc;
    kstat_t *ksp_sp, *ksp_zfs;

    void getmeminfo( void );
};


#endif
