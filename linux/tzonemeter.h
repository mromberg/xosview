//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//

#ifndef TZONEMETER_H
#define TZONEMETER_H

#include "ctzonemeter.h"

class TZoneMeter : public ComTZoneMeter {
public:
    TZoneMeter(size_t zoneNum);

    // Number of thermal zones
    static size_t count(void);

protected:
    virtual float getTemp(void);

private:
    std::string _tempFName;
};


#endif
