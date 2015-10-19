//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef CPUMETER_H
#define CPUMETER_H

#include "fieldmetergraph.h"

#include <map>


class CPUMeter : public FieldMeterGraph {
public:
    CPUMeter(unsigned int cpu=0);
    ~CPUMeter(void);

    std::string resName(void) const { return "cpu"; }
    void checkevent(void);

    void checkResources(const ResDB &rdb);

    static size_t countCPUs(void);
    static std::string cpuStr(size_t num);

private:
    unsigned int _cpu;
    std::map<pid_t, uint64_t> _ptable;

    std::vector<uint64_t> getTicks(void);
};


#endif
