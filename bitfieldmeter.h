//
//  Copyright (c) 1999, 2006, 2015 Thomas Waldmann (ThomasWaldmann@gmx.de)
//  based on work of Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef _BITFIELDMETER_H_
#define _BITFIELDMETER_H_

#include "meter.h"
#include "timer.h"
#include <string>
#include <vector>

class BitFieldMeter : public Meter {
public:
    BitFieldMeter( XOSView *parent, size_t numBits=1, size_t numfields=1,
      const std::string &title = "", const std::string &bitlegend = "",
      const std::string &fieldlegend = "", bool docaptions=false,
      bool dolegends=false, bool dousedlegends=false);
    virtual ~BitFieldMeter( void );

    virtual void drawfields(X11Graphics &g, bool manditory=false);
    void drawBits(X11Graphics &g, bool manditory=false);

    void setfieldcolor( int field, const std::string &color );
    void setfieldcolor( int field, unsigned long color);
    void reset( void );

    void setUsed (float val, float total);
    void setBits(int startbit, unsigned char values);

    void draw(X11Graphics &g);
    void checkevent( void );
    void disableMeter ( void );

    virtual void checkResources( void );

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
    int printedZeroTotalMesg_;
    int numWarnings_;

    unsigned long onColor_, offColor_;
    std::vector<char> bits_;
    std::vector<char> lastbits_;
    unsigned int numbits(void) const { return bits_.size(); }

    void SetUsedFormat ( const std::string &str );
    void drawfieldlegend(X11Graphics &g);
    void drawused(X11Graphics &g, bool manditory);
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
