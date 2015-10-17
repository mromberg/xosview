//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef METER_H
#define METER_H

#include "rdb.h"
#include "label.h"
#include "strutil.h"

class XOSView;


class Meter {
public:
    Meter( XOSView *parent, const std::string &title = "",
      const std::string &legend ="");

    virtual ~Meter( void );

    // prefix name for resouces such as cpu in cpuPriority
    virtual std::string resName(void) const = 0;
    virtual std::string name(void) const { return resName() + "Meter"; }

    virtual void checkResources(const ResDB &rdb);
    virtual void checkevent( void ) = 0;
    virtual void draw(X11Graphics &g) = 0; // Draw everything cleared
    virtual void drawIfNeeded(X11Graphics &g); // Draw if needed
    virtual void resize( int x, int y, int width, int height );

    bool requestevent(void);  // if true wants to sample at this tick
    void docaptions(bool val ) { docaptions_ = val; }
    void dolegends(bool val) { dolegends_ = val; }
    void dousedlegends(bool val) { dousedlegends_ = val; }

protected:
    XOSView *parent_;

    unsigned long fgColor(void) const { return _fgColor; }
    unsigned long bgColor(void) const { return _bgColor; }
    void title( const std::string &title ) { _title.text(title); }
    void legend( const std::string &legend, const std::string &delimiter="/" );
    bool docaptions(void) const { return docaptions_; }
    bool dolegends(void) const { return dolegends_; }
    bool dousedlegends(void) const { return dousedlegends_; }
    bool metric(void) const { return metric_; }
    void setMetric(bool val) { metric_ = val; }
    double scaleValue(double value, std::string &scale) const;
    void setLegendColor(size_t index, unsigned long color)
        { _legend.setColor(index, color); }
    virtual void drawLabels(X11Graphics &g);

    int x(void) const { return x_; }
    int y(void) const { return y_; }
    void x(int v) { x_ = v; }
    void y(int v) { y_ = v; }
    int width() const { return width_; }
    int height() const { return height_; }

private:
    int x_, y_, width_, height_;
    int priority_;
    int counter_;
    bool docaptions_, dolegends_, dousedlegends_, metric_;
    Label _title;
    MCLabel _legend;
    unsigned long _fgColor, _bgColor;
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
