//
//  Copyright (c) 1999, 2006, 2015, 2018
//  by Mike Romberg (mike-romberg@comcast.net)
//
//  This file may be distributed under terms of the GPL
//

#ifndef diskmeter_h
#define diskmeter_h

#include "perfcount.h"
#include "cdiskmeter.h"



class DiskMeter : public ComDiskMeter {
public:
    DiskMeter(void);

protected:
    virtual std::pair<double, double> getRate(void) override;

private:
    PerfQuery _query;
};


#endif
