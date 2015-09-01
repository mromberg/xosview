//
//  Copyright (c) 1999, 2006, 2015
//  by Mike Romberg (mike-romberg@comcast.net)
//
//  This file may be distributed under terms of the GPL
//

#ifndef DISKMETER_H
#define DISKMETER_H

#include "perfcount.h"
#include "fieldmetergraph.h"



class DiskMeter : public FieldMeterGraph {
public:
    DiskMeter( XOSView *parent);
    ~DiskMeter( void );

    std::string name( void ) const { return "DiskMeter"; }
    void checkevent( void );

    void checkResources(const ResDB &rdb);

private:
    float _max;
    PerfQuery _query;
};


#endif
