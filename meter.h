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

    virtual void checkResources( void );
    void title( const std::string &title ) { title_ = title; }
    const std::string &title( void ) const { return title_; }
    void legend( const std::string &legend, const std::string &delimiter="/" );
    const std::string &legend( void ) const { return legend_; }
    const std::string &legendDelimiter(void) const { return _legendDelimiter; }
    void docaptions(bool val ) { docaptions_ = val; }
    void dolegends(bool val) { dolegends_ = val; }
    void dousedlegends(bool val) { dousedlegends_ = val; }

    virtual void checkevent( void ) = 0;

    int getX() const { return x_; }
    int getY() const { return y_; }
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }

    // ----------------------------------------------------
    // Find some way to prevent anything but immediade
    // "drawing" sub classes implement these.  Possibly
    // make a meter drawing base class/interface.
    // IF YOU ARE NOT A DRAWING METER (draw lines rectanges
    // etc).  Do not call.
    // ----------------------------------------------------
    virtual void draw(X11Graphics &g) = 0;
    virtual void drawLabels(X11Graphics &g);
    virtual void drawTitle(X11Graphics &g);
    virtual void drawLegend(X11Graphics &g); // make this go away

    bool requestevent(void);  // if true wants to sample at this tick
    void resize( int x, int y, int width, int height );
    // ----------------------------------------------------

protected:
    XOSView *parent_;
    int x_, y_, width_, height_;
    int priority_, counter_;
    std::string title_, legend_;
    unsigned long textcolor_;



    double samplesPerSecond(void)
        { return 1.0 * parent_->sampleRate() / priority_; }

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
    std::string _legendDelimiter;
};


inline bool Meter::requestevent( void ){
    logAssert(priority_ != 0) << "meter " << name() << " invalid priority\n";
    counter_ = (counter_ + 1) % priority_;
    return !counter_;
}


inline void Meter::legend( const std::string &legend,
  const std::string &delimiter ) {
    legend_ = legend;
    _legendDelimiter = delimiter;
    logAssert(_legendDelimiter.size() == 1) << "delimiter not 1 char string\n";
}

#endif
