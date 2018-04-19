//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef pnetmeter_h
#define pnetmeter_h

#include "cnetmeter.h"



class PrcNetMeter : public ComNetMeter {
public:
    PrcNetMeter(void);

protected:
    virtual std::pair<float, float> getRates(void) override;

private:
    std::pair<uint64_t, uint64_t> _last;

    std::pair<uint64_t, uint64_t> getStats(void);
};


#endif
