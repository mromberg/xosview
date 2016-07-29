//
//  Copyright (c) 1999, 2006, 2015, 2016 Thomas Waldmann (ThomasWaldmann@gmx.de)
//  based on work of Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef BITFIELDMETER_H
#define BITFIELDMETER_H

#include "meter.h"
#include "timer.h"
#include "drawbits.h"


class BitFieldMeter : public Meter {
public:
    BitFieldMeter( size_t numBits=1, size_t numfields=1,
      const std::string &title = "", const std::string &bitlegend = "",
      const std::string &fieldlegend = "");

    virtual ~BitFieldMeter( void );

    // virtual from Meter
    virtual void draw(X11Graphics &g);
    virtual void drawIfNeeded(X11Graphics &g);
    virtual void checkResources(const ResDB &rdb);

protected:
    std::vector<float> fields_;
    float total_;
    std::vector<char> _bits;
    DrawBits<char> _dbits;

    size_t numfields(void) const { return fields_.size(); }
    void setfieldcolor( int field, unsigned long color);
    void reset( void );
    void setUsed (float val, float total);
    void setBits(int startbit, unsigned char values);
    unsigned int numbits(void) const { return _bits.size(); }
    virtual void drawfields(X11Graphics &g, bool mandatory=false);
    void setNumFields(int n);
    void setNumBits(int n);
    void setfieldlegend(const std::string &fieldlegend)
        { fieldLegend_ = fieldlegend; }
    void IntervalTimerStart() { _timer.start(); }
    void IntervalTimerStop() { _timer.stop(); }
    //  Before, we simply called _timer.report(), which returns usecs.
    //  However, it suffers from wrap/overflow/sign-bit problems, so
    //  instead we use doubles for everything.
    double IntervalTimeInMicrosecs() { return _timer.report_usecs(); }
    double IntervalTimeInSecs() { return _timer.report_usecs()/1e6; }

private:
    enum UsedType { INVALID_0, FLOAT, PERCENT, AUTOSCALE, INVALID_TAIL };

    float used_, lastused_;
    std::vector<int> lastvals_;
    std::vector<int> lastx_;
    std::vector<unsigned long> colors_;
    unsigned long usedcolor_;
    UsedType print_;
    bool printedZeroTotalMesg_;
    size_t numWarnings_;
    unsigned long onColor_, offColor_;
    std::string fieldLegend_;
    Timer _timer;

    void setUsedFormat ( const std::string &str );
    void drawfieldlegend(X11Graphics &g);
    void drawused(X11Graphics &g, bool mandatory);
    bool checkX(int x, int width) const;
};


#endif
