//
//  Copyright (c) 1994, 1995, 2004, 2006, 2015, 2016
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef CPUMETER_H
#define CPUMETER_H

#include "fieldmetergraph.h"



class CPUMeter : public FieldMeterGraph {
public:
    CPUMeter(size_t cpu=0);
    ~CPUMeter(void);

    std::string resName(void) const { return "cpu"; }
    void checkevent(void);

    void checkResources(const ResDB &rdb);

    static size_t countCPUs(void);
    static std::string cpuStr(size_t num);

private:
    size_t _cpu;
    std::vector<uint64_t> _oldStats;

    void getStats(std::vector<uint64_t> &stats) const;

    std::vector<std::vector<uint64_t> > readStats(void) const;
};


#endif
