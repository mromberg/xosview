//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//

#ifndef tzonemeter_h
#define tzonemeter_h

#include "ctzonemeter.h"

class TZoneMeter : public ComTZoneMeter {
public:
    TZoneMeter(size_t zoneNum);

    // Number of thermal zones
    static size_t count(void);

protected:
    virtual float getTemp(void) override;

private:
    std::string _tempFName;
};


#endif
