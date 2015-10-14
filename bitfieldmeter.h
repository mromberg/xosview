//
//  Copyright (c) 1999, 2006, 2015 Thomas Waldmann (ThomasWaldmann@gmx.de)
//  based on work of Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef BITFIELDMETER_H
#define BITFIELDMETER_H

#include "meter.h"
#include "timer.h"

#include <string>
#include <vector>

class BitFieldMeter : public Meter {
public:
    BitFieldMeter( XOSView *parent, size_t numBits=1, size_t numfields=1,
      const std::string &title = "", const std::string &bitlegend = "",
      const std::string &fieldlegend = "");

    virtual ~BitFieldMeter( void );

    // virtual from Meter
    virtual void draw(X11Graphics &g);
    virtual void checkevent( void );
    virtual void checkResources(const ResDB &rdb);

    void drawBits(X11Graphics &g, bool mandatory=false);

    void setfieldcolor( int field, const std::string &color );
    void setfieldcolor( int field, unsigned long color);
    void reset( void );

    void setUsed (float val, float total);
    void setBits(int startbit, unsigned char values);

    void disableMeter ( void );

protected:
    enum UsedType { INVALID_0, FLOAT, PERCENT, AUTOSCALE, INVALID_TAIL };

    size_t numfields(void) const { return fields_.size(); }
    std::vector<float> fields_;
    float total_, used_, lastused_;
    std::vector<int> lastvals_;
    std::vector<int> lastx_;
    std::vector<unsigned long> colors_;
    unsigned long usedcolor_;
    UsedType print_;
    bool printedZeroTotalMesg_;
    size_t numWarnings_;

    unsigned long onColor_, offColor_;
    std::vector<char> bits_;
    std::vector<char> lastbits_;
    unsigned int numbits(void) const { return bits_.size(); }

    virtual void drawfields(X11Graphics &g, bool mandatory=false);
    void setUsedFormat ( const std::string &str );
    void drawfieldlegend(X11Graphics &g);
    void drawused(X11Graphics &g, bool mandatory);
    bool checkX(int x, int width) const;

    void setNumFields(int n);
    void setNumBits(int n);
    std::string fieldLegend_;

    void setfieldlegend(const std::string &fieldlegend)
        { fieldLegend_ = fieldlegend; }

private:
    Timer _timer;
protected:
    void IntervalTimerStart() { _timer.start(); }
    void IntervalTimerStop() { _timer.stop(); }
    //  Before, we simply called _timer.report(), which returns usecs.
    //  However, it suffers from wrap/overflow/sign-bit problems, so
    //  instead we use doubles for everything.
    double IntervalTimeInMicrosecs() { return _timer.report_usecs(); }
    double IntervalTimeInSecs() { return _timer.report_usecs()/1e6; }
};

#endif
