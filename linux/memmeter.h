//
//  Copyright (c) 1994, 1995, 2006, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef memmeter_h
#define memmeter_h

#include "pmemmeter.h"



class MemMeter : public PrcMemMeter {
public:
    MemMeter(void);

    virtual void checkResources(const ResDB &rdb) override;

protected:
    virtual std::vector<LineInfo> newLineInfos(void) override;
    virtual void setFields(void) override;
};


#endif
