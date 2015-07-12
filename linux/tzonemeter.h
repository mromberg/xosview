//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//

//-------------------------------------------------------
// BASIC Data Meter Template.  See example.h for comments
//-------------------------------------------------------

#ifndef TZONEMETER_H
#define TZONEMETER_H

#include "fieldmetergraph.h"

class TZoneMeter : public FieldMeterGraph {
public:
    TZoneMeter(XOSView *parent, size_t zoneNum);
    virtual ~TZoneMeter(void);

    virtual std::string name( void ) const { return "TZoneMeter"; }
    virtual void checkevent( void );
    virtual void checkResources( void );

    // Number of thermal zones
    static size_t count(void);

private:
    float _peak;
    float _hotTrip;
    float _critTrip;
    unsigned long _normColor;
    unsigned long _hotColor;
    unsigned long _critColor;
    std::string _tempFName;
};

#endif
