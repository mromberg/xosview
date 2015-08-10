//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef CPUMETER_H
#define CPUMETER_H

#include "fieldmetergraph.h"


class CPUMeter : public FieldMeterGraph {
public:
    CPUMeter(XOSView *parent, unsigned int cpu=0);
    ~CPUMeter(void);

    std::string name(void) const { return "CPUMeter"; }
    void checkevent(void);

    void checkResources(const ResDB &rdb);

    static size_t countCPUs(void);
    static std::string cpuStr(size_t num);

private:
    unsigned int _cpu;
    std::vector<uint64_t> _lastTicks;

    std::vector<uint64_t> getTicks(void) const;
};


#endif
