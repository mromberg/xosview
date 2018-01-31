//
//  Copyright (c) 1999, 2015, 2018
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//
#ifndef memmeter_h
#define memmeter_h

#include "fieldmetergraph.h"
#include "kstat.h"


class MemMeter : public FieldMeterGraph {
public:
    MemMeter(kstat_ctl_t *kcp);

    virtual std::string resName(void) const override { return "mem"; }
    virtual void checkevent(void) override;
    virtual void checkResources(const ResDB &rdb) override;

private:
    int pageSize;
    kstat_ctl_t *kc;
    kstat_t *ksp_sp, *ksp_zfs;

    void getmeminfo(void);
};


#endif
