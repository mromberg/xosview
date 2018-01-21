//
//  Copyright (c) 2008, 2015, 2018 by Tomi Tapper <tomi.o.tapper@jyu.fi>
//
//  File based on linux/lmstemp.* by
//  Copyright (c) 2000, 2006 by Leopold Toetsch <lt@toetsch.at>
//
//  This file may be distributed under terms of the GPL
//
//
//
#ifndef coretemp_h
#define coretemp_h


#include "fieldmeter.h"



class CoreTemp : public FieldMeter {
public:
    CoreTemp(const std::string &label,
      const std::string &caption, int cpu);
    ~CoreTemp(void);

    virtual std::string resName(void) const { return "coretemp"; }
    void checkevent(void);
    void checkResources(const ResDB &rdb);

    static unsigned int countCpus(void);

private:
    int   _cpu, _cpuCount;
    float _high;
    std::vector<float> _temps;
    unsigned long _actColor, _highColor;

    void getcoretemp(void);
};


#endif
