//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef PNETMETER_H
#define PNETMETER_H

#include "cnetmeter.h"
#include "timer.h"

#include <stdint.h>



class PrcNetMeter : public ComNetMeter {
public:
    PrcNetMeter(XOSView *parent);

    virtual std::string resName(void) const { return "net"; }

protected:
    virtual std::pair<float, float> getRates(void);

private:
    Timer _timer;
    std::pair<uint64_t, uint64_t> _last;

    std::pair<uint64_t, uint64_t> getStats(void);
};


#endif
