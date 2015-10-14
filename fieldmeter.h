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
      const std::string &title = "", const std::string &legend = "");

    virtual ~FieldMeter( void );

    // virtual from Meter
    virtual void checkevent( void );
    virtual void draw(X11Graphics &g);
    virtual void drawIfNeeded(X11Graphics &g);
    virtual void checkResources(const ResDB &rdb);
    virtual void resize( int x, int y, int width, int height );

protected:
    std::vector<float> fields_;
    float total_;
    std::vector<int> lastvals_;
    std::vector<int> lastx_;

    void setNumFields(size_t n);
    size_t numfields(void) const { return fields_.size() ; }
    void setfieldcolor( int field, const std::string &color );
    void setfieldcolor( int field, unsigned long color);
    unsigned long fieldcolor(size_t field) const { return colors_[field]; }

    void setUsed (float val, float total);

    void IntervalTimerStart() { _timer.start(); }
    void IntervalTimerStop() { _timer.stop(); }
    //  Before, we simply called _timer.report(), which returns usecs.
    //  However, it suffers from wrap/overflow/sign-bit problems, so
    //  instead we use doubles for everything.
    double IntervalTimeInMicrosecs() { return _timer.report_usecs(); }
    double IntervalTimeInSecs() { return _timer.report_usecs()/1e6; }

    virtual void drawfields(X11Graphics &g, bool mandatory=false);
    bool checkX(int x, int width) const;

private:
    enum UsedType { INVALID_0, FLOAT, PERCENT, AUTOSCALE, INVALID_TAIL };

    float used_;
    std::vector<unsigned long> colors_;
    UsedType print_;
    int printedZeroTotalMesg_;
    Timer _timer;
    std::vector<float> _usedAvg;
    size_t _usedAvgIndex;
    bool _decayUsed;
    Label _used;

    void setUsedFormat ( const std::string &str );
    bool decayUsed(void) const { return _decayUsed; }
    void decayUsed(bool val) { _decayUsed = val; }
    void updateUsed(void);
};


inline bool FieldMeter::checkX(int xv, int widthv) const {
    logAssert((xv >= x()) && (xv + widthv >= x())
      && (xv <= x() + width()) && (xv + widthv <= x() + width()))
        << "bad horiz values for meter: " << name() << "\n"
        << "fields_: " << fields_ << std::endl;

    return true;
}

#endif
