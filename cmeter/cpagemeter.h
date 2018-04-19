//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef cpagemeter_h
#define cpagemeter_h

#include "fieldmetergraph.h"



class ComPageMeter : public FieldMeterGraph {
public:
    ComPageMeter(void);

    virtual std::string resName(void) const override { return "page"; }
    virtual void checkevent(void) override;

    virtual void checkResources(const ResDB &rdb) override;

protected:
    // (in, out) bytes/sec
    virtual std::pair<float, float> getPageRate(void) = 0;

private:
    float _maxspeed;
};


#endif
