//  
//  Copyright (c) 1994, 1995 by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: meter.h,v 1.7 1999/02/19 09:44:26 mcnab Exp $
//
#ifndef _METER_H_
#define _METER_H_

#define METER_H_CVSID "$Id: meter.h,v 1.7 1999/02/19 09:44:26 mcnab Exp $"

#include <stdio.h>
#include "xosview.h"	//  To grab MAX_SAMPLES_PER_SECOND.

class XOSView;

class Meter {
public:
  Meter( XOSView *parent, const char *title = "", const char *legend ="", 
	 int docaptions = 0, int dolegends = 0, int dousedlegends = 0 );
  virtual ~Meter( void );

  virtual const char *name( void ) const { return "Meter"; }
  void resize( int x, int y, int width, int height );
  virtual void checkevent( void ) = 0;
  virtual void draw( void ) = 0;
  void title( const char *title );
  const char *title( void ) { return title_; }
  void legend( const char *legend );
  const char *legend( void ) { return legend_; }
  void docaptions( int val ) { docaptions_ = val; }
  void dolegends( int val ) { dolegends_ = val; }
  void dousedlegends( int val ) { dousedlegends_ = val; }
  int requestevent( void ){ 
    if (priority_ == 0) {
      fprintf(stderr, "Warning:  meter %s had an invalid priority "
	      "of 0.  Resetting to 1...\n", name());
      priority_ = 1;
    }
    int rval = counter_ % priority_;
    counter_ = (counter_ + 1) % priority_;
    return !rval;
  }

  int getX() const { return x_; }
  int getY() const { return y_; }
  int getWidth() const { return width_; }
  int getHeight() const { return height_; }

  virtual void checkResources( void );

protected:
  XOSView *parent_;
  int x_, y_, width_, height_, docaptions_, dolegends_, dousedlegends_;
  int priority_, counter_;
  char *title_, *legend_;
  unsigned long textcolor_;
  double samplesPerSecond() { return 1.0*MAX_SAMPLES_PER_SECOND/priority_; }
  double secondsPerSample() { return 1.0/samplesPerSecond(); }

private:
};

#endif
