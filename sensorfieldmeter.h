//
//  Copyright (c) 2014, 2015 by Tomi Tapper <tomi.o.tapper@jyu.fi>
//
//  This file may be distributed under terms of the GPL
//
//  Put code common to *BSD and Linux sensor meters here.
//

#ifndef SENSORFIELDMETER_H
#define SENSORFIELDMETER_H

#include "fieldmeter.h"


class SensorFieldMeter : public FieldMeter {
public:
    SensorFieldMeter( XOSView *parent, const std::string &title = "",
      const std::string &legend = "", bool docaptions = false,
      bool dolegends = false, bool dousedlegends = false );
    ~SensorFieldMeter( void );

protected:
    std::string unit_;
    double high_, low_;
    bool has_high_, has_low_, negative_;
    unsigned long actcolor_, highcolor_, lowcolor_;

    void updateLegend( void );
    void checkFields( double low, double high );
};


#endif
