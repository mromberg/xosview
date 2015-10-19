//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef PINTRATEMETER_H
#define PINTRATEMETER_H

#include "cintratemeter.h"
#include "timer.h"

#include <stdint.h>



class PrcIrqRateMeter : public ComIrqRateMeter {
public:
    PrcIrqRateMeter(void);

protected:
    virtual float getIrqRate(void);

private:
    Timer _timer;
    uint64_t _last;

    uint64_t getIntCount(void);
};


#endif
