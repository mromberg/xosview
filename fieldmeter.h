//  
//  Copyright (c) 1994, 1995 by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: fieldmeter.h,v 1.4 1996/11/19 05:00:06 bgrayson Exp $
//
#ifndef _FIELDMETER_H_
#define _FIELDMETER_H_

#define FIELDMETER_H_CVSID "$Id: fieldmeter.h,v 1.4 1996/11/19 05:00:06 bgrayson Exp $"

#include "meter.h"

class FieldMeter : public Meter {
public:
  FieldMeter( XOSView *parent, int numfields,
	      const char *title = "", const char *legend = "", 
	      int dolegends = 0, int dousedlegends = 0 );
  virtual ~FieldMeter( void );
  
  virtual void drawfields( int manditory = 0 );
  void setfieldcolor( int field, const char *color );
  void setfieldcolor( int field, unsigned long color);
  void dolegends( int val ) { dolegends_ = val; }
  void dousedlegends( int val ) { dousedlegends_ = val; }
  void reset( void );
  void setUsed (float val) { used_ = val; }
  void used( int val ) { print_ = PERCENT; used_ = val; }
  void absolute( float val ) { print_ = FLOAT; used_ = val; }
  void absoluteK( float val ) { print_ = KBYTES; used_ = val; }
  void draw( void );
  void checkevent( void );
  void disableMeter ( void );
  
  virtual void checkResources( void );

protected:
  enum UsedType { FLOAT, PERCENT, KBYTES };

  int numfields_;
  float *fields_, total_, used_, lastused_;
  int *lastvals_, *lastx_;
  unsigned long *colors_, usedcolor_;
  UsedType print_;

  void SetUsedFormat ( const char * const str );
  void drawlegend( void );
  void drawused( int manditory );
  bool checkX(int x, int width) const;

  void setNumFields(int n);

private:
};

#endif
