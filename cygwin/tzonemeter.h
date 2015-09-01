//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//

#ifndef TZONEMETER_H
#define TZONEMETER_H

#include "perfcount.h"
#include "fieldmetergraph.h"

class TZoneMeter : public FieldMeterGraph {
public:
    TZoneMeter(XOSView *parent, size_t zoneNum);
    virtual ~TZoneMeter(void);

    virtual std::string name( void ) const { return "TZoneMeter"; }
    virtual void checkevent( void );
    virtual void checkResources(const ResDB &rdb);

    // Number of thermal zones
    static size_t count(void);

private:
    float _peak;
    float _hotTrip;
    float _critTrip;
    unsigned long _normColor;
    unsigned long _hotColor;
    unsigned long _critColor;
    PerfQuery _query;
};


#endif
