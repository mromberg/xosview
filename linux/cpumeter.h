//
//  Copyright (c) 1994, 1995, 2004, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef CPUMETER_H
#define CPUMETER_H

#include "fieldmetergraph.h"



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
    std::vector<unsigned long long> _oldStats;
    size_t _lineNum;

    void getcputime(void);
    size_t findLine(void);
    void getStats(std::vector<unsigned long long> &stats) const;
};


#endif
