//
//  Copyright (c) 2013, 2015 by Tomi Tapper ( tomi.o.tapper@student.jyu.fi )
//
//  Based on linux/btrymeter.h:
//  Copyright (c) 1997, 2005, 2006 by Mike Romberg ( mike.romberg@noaa.gov )
//
//  This file may be distributed under terms of the GPL
//

#ifndef BTRYMETER_H
#define BTRYMETER_H

#include "fieldmeter.h"



class BtryMeter : public FieldMeter {
public:
    BtryMeter( XOSView *parent );
    ~BtryMeter( void );

    virtual std::string name( void ) const { return "BtryMeter"; }
    void checkevent( void );
    void checkResources(const ResDB &rdb);

private:
    unsigned long leftcolor_, usedcolor_, chargecolor_, fullcolor_,
        lowcolor_, critcolor_, nonecolor_;
    unsigned int  old_state_;

    void getstats( void );
};


#endif
