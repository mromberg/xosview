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
    CPUMeter(XOSView *parent, const char *cpuID = "cpu");
    ~CPUMeter(void);

    std::string name(void) const { return "CPUMeter"; }
    void checkevent(void);

    void checkResources(void);

    static int countCPUs(void);
    static const char *cpuStr(int num);
protected:
    int _lineNum;
    long long cputime_[2][9];
    int cpuindex_;

    void getcputime(void);
    int findLine(const char *cpuID);
    const char *toUpper(const char *str);
private:
};

#endif
