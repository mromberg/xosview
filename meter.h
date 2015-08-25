//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef _METER_H_
#define _METER_H_

#include "xosview.h"	//  To grab MAX_SAMPLES_PER_SECOND.
#include "label.h"
#include "strutil.h"

#include <string>




class Meter {
public:
    Meter( XOSView *parent, const std::string &title = "",
      const std::string &legend ="", bool docaptions=false,
      bool dolegends=false, bool dousedlegends=false);

    virtual ~Meter( void );

    virtual std::string name( void ) const { return "Meter"; }

    virtual void checkResources(const ResDB &rdb);
    void title( const std::string &title ) { _title.text(title); }
    void legend( const std::string &legend, const std::string &delimiter="/" );
    void docaptions(bool val ) { docaptions_ = val; }
    void dolegends(bool val) { dolegends_ = val; }
    void dousedlegends(bool val) { dousedlegends_ = val; }

    virtual void checkevent( void ) = 0;

    int getX() const { return x_; }
    int getY() const { return y_; }
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }

    virtual void draw(X11Graphics &g) = 0; // Draw everything cleared
    virtual void drawIfNeeded(X11Graphics &g); // Draw if needed

    bool requestevent(void);  // if true wants to sample at this tick
    void resize( int x, int y, int width, int height );
    // ----------------------------------------------------

protected:
    XOSView *parent_;
    int x_, y_, width_, height_;
    int priority_, counter_;
    unsigned long textcolor_;


    virtual void drawLabels(X11Graphics &g);

    double samplesPerSecond(void)
        { return 1.0 * parent_->sampleRate() / priority_; }

    double secondsPerSample() { return 1.0/samplesPerSecond(); }

    bool docaptions(void) const { return docaptions_; }
    bool dolegends(void) const { return dolegends_; }
    bool dousedlegends(void) const { return dousedlegends_; }
    bool metric(void) const { return metric_; }
    void setMetric(bool val) { metric_ = val; }
    double scaleValue(double value, std::string &scale) const;
    void setLegendColor(size_t index, unsigned long color)
        { _legend.setColor(index, color); }

private:
    bool docaptions_, dolegends_, dousedlegends_, metric_;
    Label _title;
    MCLabel _legend;
};


inline bool Meter::requestevent( void ){
    logAssert(priority_ != 0) << "meter " << name() << " invalid priority\n";
    counter_ = (counter_ + 1) % priority_;
    return !counter_;
}


inline void Meter::legend( const std::string &legend,
  const std::string &delimiter ) {
    _legend.text(legend);
    _legend.delimiter(delimiter);
}


#endif
