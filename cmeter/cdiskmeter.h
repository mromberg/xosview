//
//  Copyright (c) 1999, 2006, 2015
//  by Mike Romberg (mike-romberg@comcast.net)
//
//  This file may be distributed under terms of the GPL
//

#ifndef CDISKMETER_H
#define CDISKMETER_H

#include "fieldmetergraph.h"



class ComDiskMeter : public FieldMeterGraph {
public:
    ComDiskMeter( XOSView *parent );
    ~ComDiskMeter( void );

    std::string resName( void ) const { return "disk"; }
    void checkevent( void );

    void checkResources(const ResDB &rdb);

protected:
    // bytes/sec (read, write)
    virtual std::pair<double, double> getRate(void) = 0;

private:
    float maxspeed_;
};


#endif
