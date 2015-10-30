//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
#ifndef EXAMPLE_H
#define EXAMPLE_H

#include "fieldmetergraph.h"  // our parent drawing class

class ExampleMeter : public FieldMeterGraph {
public:
    ExampleMeter(void);
    virtual ~ExampleMeter(void);

    virtual std::string resName( void ) const { return "wcore"; }
    virtual void checkevent( void );
    virtual void checkResources(const ResDB &rdb);

private:
    unsigned long long _warpCoreTemp;   // current value
    unsigned long long _testMaximum;    // test max temp
    unsigned long long _designMaximum;  // design max temp
    // colors
    unsigned long _normColor, _warnColor, _alarmColor;

    void readWarpCoreTemp(void);        // read the current temp
};

//----------  .cc  -------------------------------------------------------

inline ExampleMeter::ExampleMeter(void)
    : FieldMeterGraph(2, "WCOR", "TEMP/USED"),
      _warpCoreTemp(0), _testMaximum(0), _designMaximum(0),
      _normColor(0), _warnColor(0), _alarmColor(0) {

    readWarpCoreTemp();
}


inline ExampleMeter::~ExampleMeter(void) {
}


inline void ExampleMeter::checkResources(const ResDB &rdb) {

    FieldMeterGraph::checkResources(rdb);

    _testMaximum = util::stoi(rdb.getResourceOrUseDefault(
          "exampleTestMax", "500"));
    _designMaximum = util::stoi(rdb.getResourceOrUseDefault(
          "exampleDesignMax", "550"));

    setfieldcolor(1, rdb.getColor("warpBG", "blue"));

    _normColor = rdb.getColor("warpColor", "green");
    _warnColor = rdb.getColor("warpWarnColor", "yellow");
    _alarmColor = rdb.getColor("warpAlarmColor", "red");
    setfieldcolor(0, _normColor);
}


inline void ExampleMeter::checkevent( void ) {

    readWarpCoreTemp();

    float percentVal = static_cast<float>(_warpCoreTemp)
        / static_cast<float>(_testMaximum);
    total_ = 1.0;
    fields_[0] = percentVal;
    if (fields_[0] > 1.0) // peak the meter
        fields_[0] = 1.0;
    fields_[1] = 1.0 - fields_[0];

    // Change the field color to show our level of concern
    if (percentVal > 0.9)
        setfieldcolor(0, _alarmColor);
    else if (percentVal > 0.75)
        setfieldcolor(0, _warnColor);
    else
        setfieldcolor(0, _normColor);

    setUsed(percentVal, total_);
}


inline void ExampleMeter::readWarpCoreTemp(void) {

    static size_t count = 0;

    count++;

    _warpCoreTemp = count * 10;

    if (_warpCoreTemp > _designMaximum + 30) // Q hits reset switch
        count = 0;
}

#endif
