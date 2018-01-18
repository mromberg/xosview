//
//  Copyright (c) 2014, 2015, 2018 by Tomi Tapper <tomi.o.tapper@jyu.fi>
//
//  This file may be distributed under terms of the GPL
//
//  Put code common to *BSD and Linux sensor meters here.
//

#ifndef sensorfieldmeter_h
#define sensorfieldmeter_h

#include "fieldmeter.h"


class SensorFieldMeter : public FieldMeter {
public:
    SensorFieldMeter(const std::string &title="", const std::string &legend="");

    virtual ~SensorFieldMeter(void);

protected:
    double _high, _low;
    bool _hasHigh, _hasLow;
    unsigned long _actColor, _highColor, _lowColor;

    const std::string &unit(void) const { return _unit; }
    void updateLegend(void);
    void checkFields(double low, double high);

private:
    bool _negative;
    std::string _unit;
};


#endif
