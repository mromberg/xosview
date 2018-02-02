//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef cpumeter_h
#define cpumeter_h

#include "fieldmetergraph.h"
#include "xosvproc.h"

#include <map>


class CPUMeter : public FieldMeterGraph {
public:
    CPUMeter(unsigned int cpu=0);

    virtual std::string resName(void) const override { return "cpu"; }
    virtual void checkevent(void) override;

    virtual void checkResources(const ResDB &rdb) override;

    static size_t countCPUs(void);
    static std::string cpuStr(size_t num);

private:
    unsigned int _cpu;
    std::map<pid_t, XOSVProc> _ptable;

    std::vector<uint64_t> getTicks(void);
};


#endif
