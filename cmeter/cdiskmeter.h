//
//  Copyright (c) 1999, 2006, 2015, 2018
//  by Mike Romberg (mike-romberg@comcast.net)
//
//  This file may be distributed under terms of the GPL
//

#ifndef cdiskmeter_h
#define cdiskmeter_h

#include "fieldmetergraph.h"



class ComDiskMeter : public FieldMeterGraph {
public:
    ComDiskMeter(void);

    virtual std::string resName(void) const override { return "disk"; }
    virtual void checkevent(void) override;
    virtual void checkResources(const ResDB &rdb) override;

protected:
    // bytes/sec (read, write)
    virtual std::pair<double, double> getRate(void) = 0;

private:
    float _maxspeed;
};


#endif
