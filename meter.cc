//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "general.h"
#include "meter.h"
#include "xosview.h"

Meter::Meter( XOSView *parent, const std::string &title,
  const std::string &legend)
    : parent_(parent), x_(0), y_(0), width_(1), height_(1),
      priority_(1), counter_(0),
      docaptions_(false),
      dolegends_(false), dousedlegends_(false), metric_(false),
      _title(x_, y_, title, Label::BLSW),
      _legend(x_, y_, legend, "/") {

    resize( parent->xoff(), parent->newypos(), parent->width() - 10, 10 );
}


Meter::~Meter( void ){
}


void Meter::checkResources(const ResDB &rdb) {
    priority_ = util::stoi(rdb.getResourceOrUseDefault(resName() + "Priority",
          "10"));
    unsigned long tcolor = rdb.getColor("meterLabelColor");
    _title.color(tcolor);
    _legend.color(tcolor);
}


void Meter::resize( int x, int y, int width, int height ){
    x_ = x;
    y_ = y;
    width_ = (width>=0) ? width : 0;    // fix for cosmetical bug:
    height_ = (height>=0) ? height : 0; // beware of values < 0 !
    width_ &= ~1;                       // only allow even width_ values
    _title.move(0, y_ + height_ + 2);
    _legend.move(x_, y_ - 1);
}


void Meter::drawLabels(X11Graphics &g) {
    if (dolegends()) {
        _title.draw(g);

        if (docaptions())
            _legend.draw(g);
    }
}


void Meter::drawIfNeeded(X11Graphics &g) {
    if (dolegends()) {
        _title.drawIfNeeded(g);

        if (docaptions())
            _legend.drawIfNeeded(g);
    }
}


double Meter::scaleValue(double value, std::string &scale) const {
    double scaled = ( value < 0 ? -value : value );
    scale = "";

    if (scaled >= 999.5*1e15){
        scale = "E";
        scaled = value / ( metric() ? 1e18 : 1ULL<<60 );
    }
    else if (scaled >= 999.5*1e12){
        scale = "P";
        scaled = value / ( metric() ? 1e15 : 1ULL<<50 );
    }
    else if (scaled >= 999.5*1e9){
        scale = "T";
        scaled = value / ( metric() ? 1e12 : 1ULL<<40 );
    }
    else if (scaled >= 999.5*1e6){
        scale = "G";
        scaled = value / ( metric() ? 1e9 : 1UL<<30 );
    }
    else if (scaled >= 999.5*1e3){
        scale = "M";
        scaled = value / ( metric() ? 1e6 : 1UL<<20 );
    }
    else if (scaled >= 999.5){
        scale = ( metric() ? "k" : "K" );
        scaled = value / ( metric() ? 1e3 : 1UL<<10 );
    }
    else if (scaled < 0.9995 && metric()){
        if (scaled >= 0.9995/1e3){
            scale = "m";
            scaled = value * 1e3;
        }
        else if (scaled >= 0.9995/1e6){
            scale = "\265";
            scaled = value * 1e6;
        }
        else {
            scale = "n";
            scaled = value * 1e9;
        }
        // add more if needed
    }
    else {
        scaled = value;
    }

    return scaled;
}
