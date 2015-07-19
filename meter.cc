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
  const std::string &legend, bool docaptions, bool dolegends,
  bool dousedlegends )
    : parent_(parent), x_(0), y_(0), width_(1), height_(1),
      priority_(1), counter_(0), title_(title), legend_(legend),
      textcolor_(0), docaptions_(docaptions),
      dolegends_(dolegends), dousedlegends_(dousedlegends), metric_(false) {

    resize( parent->xoff(), parent->newypos(), parent->width() - 10, 10 );
}

Meter::~Meter( void ){
}

void Meter::checkResources( void ){
    textcolor_ = parent_->g().allocColor(parent_->getResource(
          "meterLabelColor"));
}

void Meter::resize( int x, int y, int width, int height ){
    x_ = x;
    y_ = y;
    width_ = (width>=0) ? width : 0;    // fix for cosmetical bug:
    height_ = (height>=0) ? height : 0; // beware of values < 0 !
    width_ &= ~1;                       // only allow even width_ values
}

bool Meter::requestevent( void ){
    if (priority_ == 0) {
        logBug << "meter " << name()
               << " had an invalid priority"
               << " of 0.  Resetting to 1..." << std::endl;
        priority_ = 1;
    }
    int rval = counter_ % priority_;
    counter_ = (counter_ + 1) % priority_;
    return !rval;
}

void Meter::drawLabels(X11Graphics &g) {
    drawTitle(g);
    drawLegend(g);
}

void Meter::drawLegend(X11Graphics &g) {
    if (dolegends() && docaptions()) {
        int x = x_;
        int y = y_ - 1 - g.textDescent() - 1; // the bonus -1 = mystery
        // think we always have full window clear here
        g.setFG( textcolor_ );
        g.drawString( x, y, legend_ );
    }
}

void Meter::drawTitle(X11Graphics &g) {
    if (dolegends()) {
        g.setFG( textcolor_ );
        g.drawString( 0, y_ + height_ + 1, title_ );
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
