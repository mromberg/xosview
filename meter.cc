//  
//  Copyright (c) 1994, 1995 by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: meter.cc,v 1.4 1998/09/18 15:38:55 bgrayson Exp $
//
#include "general.h"
#include "meter.h"
#include "xosview.h"

CVSID("$Id: meter.cc,v 1.4 1998/09/18 15:38:55 bgrayson Exp $");
CVSID_DOT_H(METER_H_CVSID);

Meter::Meter( XOSView *parent, const char *title, const char *legend, 
              int dolegends, int dousedlegends ) {
  title_ = legend_ = NULL;
  Meter::title( title );
  Meter::legend( legend );
  parent_ = parent;
  dolegends_ = dolegends;
  dousedlegends_ = dousedlegends;
  priority_ = 1;
  counter_ = 0;
  resize( parent->xoff(), parent->newypos(), parent->width() - 10, 10 );

}

Meter::~Meter( void ){
  delete[] title_;
  delete[] legend_;
}

void Meter::checkResources( void ){
  textcolor_ = parent_->allocColor( parent_->getResource( "meterLabelColor") );
}

void Meter::title( const char *title ){
  delete[] title_;
  int len = strlen(title);
  title_ = new char[len + 1];
  strncpy( title_, title, len );
}

void Meter::legend( const char *legend ){
  delete[] legend_;
  int len = strlen(legend);
  legend_ = new char[len + 1];
  strncpy( legend_, legend, len );
}

void Meter::resize( int x, int y, int width, int height ){
  x_ = x;
  y_ = y;
  width_ = width;
  height_ = height;
}


