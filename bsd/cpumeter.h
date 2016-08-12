//
//  Copyright (c) 1994, 1995, 2015, 2016
//  by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  This file may be distributed under terms of the GPL.
//
#ifndef CPUMETER_H
#define CPUMETER_H

#include "fieldmetergraph.h"


class CPUMeter : public FieldMeterGraph {
public:
    CPUMeter( unsigned int nbr );
    ~CPUMeter( void );

    virtual std::string resName( void ) const { return "cpu"; }
    void checkevent( void );
    void checkResources(const ResDB &rdb);

    static size_t countCPUs(void);

protected:
    void getcputime( void );

private:
    std::vector<std::vector<uint64_t> > _cputime;
    unsigned int _cpuindex, _nbr;

    static void getCPUTimes(std::vector<uint64_t> &timesArray, size_t cpu = 0);
};


#endif
