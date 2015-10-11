//
//  Copyright (c) 2015
//  by Mike Romberg (mike-romberg@comcast.net)
//
//  This file may be distributed under terms of the GPL
//

#ifndef PDISKMETER_H
#define PDISKMETER_H

#include "cdiskmeter.h"

#include <stdint.h>



class PrcDiskMeter : public ComDiskMeter {
public:
    PrcDiskMeter(XOSView *parent);
    ~PrcDiskMeter(void);

    virtual std::string resName(void) const { return "disk"; }

protected:
    virtual std::pair<double, double> getRate(void);

private:
    std::pair<uint64_t, uint64_t> _last;
    Timer _timer;

    std::pair<uint64_t, uint64_t> getTotals(void);
    std::pair<uint64_t, uint64_t> getSysTotals(void) const;
    std::pair<uint64_t, uint64_t> getPrcTotals(void) const;
};

#endif
