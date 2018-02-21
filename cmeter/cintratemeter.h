//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#ifndef cintratemeter_h
#define cintratemeter_h

#include "fieldmetergraph.h"


class ComIrqRateMeter : public FieldMeterGraph {
public:
    ComIrqRateMeter(void);

    virtual std::string resName(void) const override { return "irqrate"; }
    virtual void checkevent(void) override;

    virtual void checkResources(const ResDB &rdb) override;

protected:
    virtual float getIrqRate(void) = 0; // irqs / sec

private:
    float _peak;
};


#endif
