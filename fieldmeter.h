//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef FIELDMETER_H
#define FIELDMETER_H
#include "meter.h"
#include "timer.h"

#include <string>
#include <vector>

class FieldMeter : public Meter {
public:
    FieldMeter( XOSView *parent, size_t numfields,
      const std::string &title = "", const std::string &legend = "",
      bool docaptions=false, bool dolegends=false, bool dousedlegends=false);
    virtual ~FieldMeter( void );

    // virtual from Meter
    virtual void checkevent( void );
    virtual void draw(X11Graphics &g);
    virtual void checkResources( void );

    // Virtual starting here.
    virtual void drawfields(X11Graphics &g, bool manditory=false);

    size_t numfields(void) const { return fields_.size() ; }
    void setfieldcolor( int field, const std::string &color );
    void setfieldcolor( int field, unsigned long color);
    void reset( void );

    void setUsed (float val, float total);


    void disableMeter ( void );

protected:
    enum UsedType { INVALID_0, FLOAT, PERCENT, AUTOSCALE, INVALID_TAIL };

    std::vector<float> fields_;
    float total_, used_;
    std::vector<int> lastvals_;
    std::vector<int> lastx_;
    std::vector<unsigned long> colors_;
    unsigned long usedcolor_;
    UsedType print_;
    int printedZeroTotalMesg_;
    int numWarnings_;

    void setUsedFormat ( const std::string &str );
    bool decayUsed(void) const { return _decayUsed; }
    void decayUsed(bool val) { _decayUsed = val; }
    virtual void drawLegend(X11Graphics &g);
    void drawused(X11Graphics &g, bool manditory);

    bool checkX(int x, int width) const;

    void setNumFields(size_t n);

    void IntervalTimerStart() { _timer.start(); }
    void IntervalTimerStop() { _timer.stop(); }
    //  Before, we simply called _timer.report(), which returns usecs.
    //  However, it suffers from wrap/overflow/sign-bit problems, so
    //  instead we use doubles for everything.
    double IntervalTimeInMicrosecs() { return _timer.report_usecs(); }
    double IntervalTimeInSecs() { return _timer.report_usecs()/1e6; }

private:
    Timer _timer;
    std::string _lastUsedStr;
    std::vector<float> _usedAvg;
    size_t _usedAvgIndex;
    bool _decayUsed;
};

#endif
