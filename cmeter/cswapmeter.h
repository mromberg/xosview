//
//  Copyright (c) 1994, 1995, 2006, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef cswapmeter_h
#define cswapmeter_h


#include "fieldmetergraph.h"



class ComSwapMeter : public FieldMeterGraph {
public:
    ComSwapMeter(void);

    virtual std::string resName(void) const override { return "swap"; }
    virtual void checkevent(void) override;

    virtual void checkResources(const ResDB &rdb) override;

protected:
    // first=total, second=free
    virtual std::pair<uint64_t, uint64_t> getswapinfo(void) = 0;
};


#endif
