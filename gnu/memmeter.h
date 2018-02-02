//
//  Copyright (c) 1994, 1995, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//  2007 by Samuel Thibault ( samuel.thibault@ens-lyon.org )
//
//  This file may be distributed under terms of the GPL
//
#ifndef memmeter_h
#define memmeter_h

#include "fieldmetergraph.h"



class MemMeter : public FieldMeterGraph {
public:
    MemMeter(void);

    virtual std::string resName(void) const override { return "mem"; }
    virtual void checkevent(void) override;

    virtual void checkResources(const ResDB &rdb) override;
};

#endif
