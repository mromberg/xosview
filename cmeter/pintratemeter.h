//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef pintratemeter_h
#define pintratemeter_h

#include "cintratemeter.h"



class PrcIrqRateMeter : public ComIrqRateMeter {
public:
    PrcIrqRateMeter(void);

protected:
    virtual float getIrqRate(void) override;

private:
    uint64_t _last;

    uint64_t getIntCount(void);
};


#endif
