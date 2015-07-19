//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef _METER_H_
#define _METER_H_

#include "xosview.h"	//  To grab MAX_SAMPLES_PER_SECOND.
#include "strutil.h"

#include <string>



class Meter {
public:
    Meter( XOSView *parent, const std::string &title = "",
      const std::string &legend ="",
      bool docaptions=false, bool dolegends=false, bool dousedlegends=false);
    virtual ~Meter( void );

    virtual std::string name( void ) const { return "Meter"; }
    virtual void checkevent( void ) = 0;
    virtual void draw(X11Graphics &g) = 0;
    virtual void drawLabels(X11Graphics &g);
    virtual void drawTitle(X11Graphics &g);
    virtual void drawLegend(X11Graphics &g);
    virtual void checkResources( void );


    void resize( int x, int y, int width, int height );
    void title( const std::string &title ) { title_ = title; }
    const std::string &title( void ) const { return title_; }
    void legend( const std::string &legend ) { legend_ = legend; }
    const std::string &legend( void ) const { return legend_; }
    void docaptions(bool val ) { docaptions_ = val; }
    void dolegends(bool val) { dolegends_ = val; }
    void dousedlegends(bool val) { dousedlegends_ = val; }
    bool requestevent(void);

    int getX() const { return x_; }
    int getY() const { return y_; }
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }



protected:
    XOSView *parent_;
    int x_, y_, width_, height_;
    int priority_, counter_;
    std::string title_, legend_;
    unsigned long textcolor_;
    double samplesPerSecond(void)
        { return 1.0 * XOSView::maxSampRate() / priority_; }

    double secondsPerSample() { return 1.0/samplesPerSecond(); }

    bool docaptions(void) const { return docaptions_; }
    bool dolegends(void) const { return dolegends_; }
    bool dousedlegends(void) const { return dousedlegends_; }
    bool metric(void) const { return metric_; }
    void setMetric(bool val) { metric_ = val; }
    double scaleValue(double value, std::string &scale) const;

private:
    // Child classes were caught creating their own
    // setters for these.  So...
    bool docaptions_, dolegends_, dousedlegends_, metric_;
};

#endif
