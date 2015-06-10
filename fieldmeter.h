//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef _FIELDMETER_H_
#define _FIELDMETER_H_

#include "meter.h"
#include "timer.h"
#include <string>
#include <vector>

class FieldMeter : public Meter {
public:
    FieldMeter( XOSView *parent, int numfields,
      const std::string &title = "", const std::string &legend = "",
      int docaptions = 0, int dolegends = 0, int dousedlegends = 0 );
    virtual ~FieldMeter( void );

    size_t numfields(void) const { return fields_.size() ; }
    virtual void drawfields( int manditory = 0 );
    void setfieldcolor( int field, const std::string &color );
    void setfieldcolor( int field, unsigned long color);
    void docaptions( int val ) { docaptions_ = val; }
    void dolegends( int val ) { dolegends_ = val; }
    void dousedlegends( int val ) { dousedlegends_ = val; }
    void reset( void );

    void setUsed (float val, float total);
    void draw( void );
    void checkevent( void );
    void disableMeter ( void );

    virtual void checkResources( void );

protected:
    enum UsedType { INVALID_0, FLOAT, PERCENT, AUTOSCALE, INVALID_TAIL };

    std::vector<float> fields_;
    float total_, used_, lastused_;
    std::vector<int> lastvals_;
    std::vector<int> lastx_;
    std::vector<unsigned long> colors_;
    unsigned long usedcolor_;
    UsedType print_;
    int printedZeroTotalMesg_;
    int numWarnings_;

    void setUsedFormat ( const std::string &str );
    void drawlegend( void );
    void drawused( int manditory );
    bool checkX(int x, int width) const;

    void setNumFields(int n);


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
