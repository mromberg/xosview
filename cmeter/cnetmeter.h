//
//  Copyright (c) 1994, 1995, 2006, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef cnetmeter_h
#define cnetmeter_h

#include "fieldmetergraph.h"



class ComNetMeter : public FieldMeterGraph {
public:
    ComNetMeter(void);

    virtual std::string resName(void) const override { return "net"; }
    virtual void checkevent(void) override;

    virtual void checkResources(const ResDB &rdb) override;

protected:
    // bytes/sec (in, out)
    virtual std::pair<float, float> getRates(void) = 0;

private:
    float _maxBandwidth;  // in bytes/sec
};


#endif
