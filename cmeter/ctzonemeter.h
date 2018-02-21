//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//

#ifndef ctzonemeter_h
#define ctzonemeter_h

#include "fieldmetergraph.h"



class ComTZoneMeter : public FieldMeterGraph {
public:
    ComTZoneMeter(size_t zoneNum);

    virtual std::string resName( void ) const override { return "tzone"; }
    virtual void checkevent(void) override;
    virtual void checkResources(const ResDB &rdb) override;

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
