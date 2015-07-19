//
//  Copyright (c) 2012, 2015 by Tomi Tapper <tomi.o.tapper@jyu.fi>
//
//  File based on linux/lmstemp.* by
//  Copyright (c) 2000, 2006 by Leopold Toetsch <lt@toetsch.at>
//
//  This file may be distributed under terms of the GPL
//
//
//
#ifndef BSDSENSOR_H
#define BSDSENSOR_H

#include "sensorfieldmeter.h"


#define NAMESIZE 32


class BSDSensor : public SensorFieldMeter {
public:
    BSDSensor( XOSView *parent, const std::string &name,
      const std::string &high,
      const std::string &low, const std::string &label,
      const std::string &caption, int nbr );
    ~BSDSensor( void );

    virtual std::string name( void ) const { return "BSDSensor"; }
    void checkevent( void );
    void checkResources( void );

protected:
    void getsensor( void );

private:
    std::string name_, val_, highname_, lowname_, highval_, lowval_;
    int nbr_;
};


#endif
