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

private:
    std::vector<std::vector<uint64_t> > _cputime;
    size_t _cpuindex, _nbr;

    const std::vector<uint64_t> &getStats(void) const;
    std::vector<std::vector<uint64_t> > readStats(void) const;
};


#endif
