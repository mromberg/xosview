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
    : parent_(parent), priority_(1), counter_(0),
      title_(title), legend_(legend),docaptions_(docaptions),
      dolegends_(dolegends), dousedlegends_(dousedlegends) {

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
