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
    BitFieldMeter( XOSView *parent, int numBits = 1, int numfields = 1,
      const std::string &title = "", const std::string &bitlegend = "",
      const std::string &fieldlegend = "", int docaptions = 0,
      int dolegends = 0, int dousedlegends = 0 );
    virtual ~BitFieldMeter( void );

    virtual void drawfields( int manditory = 0 );
    virtual void drawfieldsNewG(X11Graphics &g, int manditory = 0);
    void drawBits(X11Graphics &g, int manditory = 0 );

    void setfieldcolor( int field, const std::string &color );
    void setfieldcolor( int field, unsigned long color);
    void docaptions( int val ) { docaptions_ = val; }
    void dolegends( int val ) { dolegends_ = val; }
    void dousedlegends( int val ) { dousedlegends_ = val; }
    void reset( void );

    void setUsed (float val, float total);
    void setBits(int startbit, unsigned char values);

    void draw( void );
    void drawNewG(X11Graphics &g);
    void checkevent( void );
    void disableMeter ( void );

    virtual void checkResources( void );

protected:
    enum UsedType { INVALID_0, FLOAT, PERCENT, AUTOSCALE, INVALID_TAIL };

    unsigned int numfields(void) const { return fields_.size(); }
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
    void drawfieldlegend( void );
    void drawfieldlegendNewG(X11Graphics &g);
    void drawused( int manditory );
    void drawusedNewG(X11Graphics &g, int manditory);
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
