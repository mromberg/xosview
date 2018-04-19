//
//  Copyright (c) 1994, 1995, 2004, 2006, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef cpumeter_h
#define cpumeter_h

#include "fieldmetergraph.h"



class CPUMeter : public FieldMeterGraph {
public:
    CPUMeter(size_t cpu=0);

    virtual std::string resName(void) const override { return "cpu"; }
    virtual void checkevent(void) override;

    virtual void checkResources(const ResDB &rdb) override;

    static size_t countCPUs(void);
    static std::string cpuStr(size_t num);

private:
    size_t _cpu;
    std::vector<unsigned long long> _oldStats;
    size_t _lineNum;

    void getcputime(void);
    size_t findLine(void);
    void getStats(std::vector<unsigned long long> &stats) const;
};


#endif
