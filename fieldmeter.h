//
//  Copyright (c) 1994, 1995, 2006, 2015, 2016, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef fieldmeter_h
#define fieldmeter_h

#include "meter.h"
#include "timer.h"
#include "log.h"
#include "strutil.h"



class FieldMeter : public Meter {
public:
    FieldMeter(size_t numfields, const std::string &title="",
      const std::string &legend="");

    virtual void draw(X11Graphics &g) override;
    virtual void drawIfNeeded(X11Graphics &g) override;
    virtual void checkResources(const ResDB &rdb) override;
    virtual void resize(int x, int y, int width, int height) override;

protected:
    std::vector<float> _fields;
    float _total;
    std::vector<int> _lastvals;
    std::vector<int> _lastx;

    void setNumFields(size_t n);
    size_t numfields(void) const { return _fields.size() ; }
    void setfieldcolor(size_t field, unsigned long color);
    unsigned long fieldcolor(size_t field) const { return _colors[field]; }

    void setUsed (float val, float total);

    void IntervalTimerStart(void) { _timer.start(); }
    void IntervalTimerStop(void) { _timer.stop(); }
    //  Before, we simply called _timer.report(), which returns usecs.
    //  However, it suffers from wrap/overflow/sign-bit problems, so
    //  instead we use doubles for everything.
    double IntervalTimeInMicrosecs(void) { return _timer.report_usecs(); }
    double IntervalTimeInSecs(void) { return _timer.report_usecs() / 1e6; }

    bool checkX(int x, int width) const;

    virtual void drawfields(X11Graphics &g, bool mandatory=false);

    // Values used to draw the fields only.
    int fldx(void) const { return x(); }
    int fldwidth(void) const { return width(); }

private:
    enum UsedType { INVALID_0, FLOAT, PERCENT, AUTOSCALE, INVALID_TAIL };

    float _used;
    std::vector<unsigned long> _colors;
    UsedType _usedFmt;
    bool _printedZeroTotalMsg;
    Timer _timer;
    std::vector<float> _usedAvg;
    size_t _usedAvgIndex;
    bool _decayUsed;
    Label _usedLabel;

    void setUsedFormat(const std::string &str);
    void updateUsed(void);
};


inline bool FieldMeter::checkX(int xv, int widthv) const {
    logAssert((xv >= x()) && (xv + widthv >= x())
      && (xv <= x() + width()) && (xv + widthv <= x() + width()))
        << "bad horiz values for meter: " << name() << "\n"
        << "_fields: " << _fields << std::endl;

    return true;
}

#endif
