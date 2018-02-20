//
//  Copyright (c) 2015, 2018
//  by Mike Romberg (mike-romberg@comcast.net)
//
//  This file may be distributed under terms of the GPL
//

#ifndef pdiskmeter_h
#define pdiskmeter_h

#include "cdiskmeter.h"

#include <cstdint>



class PrcDiskMeter : public ComDiskMeter {
public:
    PrcDiskMeter(void);

protected:
    virtual std::pair<double, double> getRate(void) override;

private:
    std::pair<uint64_t, uint64_t> _last;

    std::pair<uint64_t, uint64_t> getTotals(void);
    std::pair<uint64_t, uint64_t> getSysTotals(void) const;
    std::pair<uint64_t, uint64_t> getPrcTotals(void) const;
};

#endif
