//
//  Copyright (c) 2015, 2018
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//

#ifndef cpumeter_h
#define cpumeter_h

#include "fieldmetergraph.h"
#include "kstats.h"



class CPUMeter : public FieldMeterGraph {
public:
    CPUMeter(kstat_ctl_t *kcp, int cpuid=0);

    virtual std::string resName(void) const override { return "cpu"; }
    virtual void checkevent(void) override;
    virtual void checkResources(const ResDB &rdb) override;

    static std::string cpuStr(int num);

private:
    std::vector<std::vector<float>> _cputime;
    size_t _cpuindex;
    KStatList *_cpustats;
    bool _aggregate;
    kstat_ctl_t *_kc;
    kstat_t *_ksp;

    void getcputime(void);
};


#endif
