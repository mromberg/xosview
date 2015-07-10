//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#ifndef IRQRATEMETER_H
#define IRQRATEMETER_H

#include "fieldmetergraph.h"
#include "timer.h"


class IrqRateMeter : public FieldMeterGraph {
public:
    IrqRateMeter( XOSView *parent );
    ~IrqRateMeter( void );

    virtual std::string name( void ) const { return "IrqRateMeter"; }
    void checkevent( void );

    void checkResources( void );

private:
    unsigned long long _peak;
    unsigned long long _last;
    Timer _timer;

    unsigned long long getIntCount(void) const;
};

#endif
