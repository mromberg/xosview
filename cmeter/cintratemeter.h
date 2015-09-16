//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#ifndef CINTRATEMETER_H
#define CINTRATEMETER_H

#include "fieldmetergraph.h"


class ComIrqRateMeter : public FieldMeterGraph {
public:
    ComIrqRateMeter( XOSView *parent );
    ~ComIrqRateMeter( void );

    virtual std::string name( void ) const { return "IrqRateMeter"; }
    void checkevent( void );

    void checkResources(const ResDB &rdb);

protected:
    virtual float getIrqRate(void) = 0; // irqs / sec

private:
    float _peak;
};


#endif
