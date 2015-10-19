//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//

#ifndef CTZONEMETER_H
#define CTZONEMETER_H

#include "fieldmetergraph.h"

class ComTZoneMeter : public FieldMeterGraph {
public:
    ComTZoneMeter(size_t zoneNum);
    virtual ~ComTZoneMeter(void);

    virtual std::string resName( void ) const { return "tzone"; }
    virtual void checkevent( void );
    virtual void checkResources(const ResDB &rdb);

protected:
    virtual float getTemp(void) = 0;  // degrees Celsius

private:
    float _peak;
    float _hotTrip;
    float _critTrip;
    unsigned long _normColor;
    unsigned long _hotColor;
    unsigned long _critColor;
};


#endif
