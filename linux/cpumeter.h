//
//  Copyright (c) 1994, 1995, 2004, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef _CPUMETER_H_
#define _CPUMETER_H_

#include "fieldmetergraph.h"

class CPUMeter : public FieldMeterGraph {
public:
    CPUMeter(XOSView *parent, const std::string &cpuID = "cpu");
    ~CPUMeter(void);

    std::string name(void) const { return "CPUMeter"; }
    void checkevent(void);

    void checkResources(void);

    static size_t countCPUs(void);
    static std::string cpuStr(size_t num);
protected:
    int _lineNum;
    long long cputime_[2][9];
    int cpuindex_;

    void getcputime(void);
    int findLine(const std::string &cpuID);
private:
};

#endif
