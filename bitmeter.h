//  
//  Copyright (c) 1994, 1995 by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: bitmeter.h,v 1.3 1996/11/19 02:17:29 bgrayson Exp $
//
#ifndef _BITMETER_H_
#define _BITMETER_H_

#define BITMETER_H_CVSID "$Id: bitmeter.h,v 1.3 1996/11/19 02:17:29 bgrayson Exp $"

#include "meter.h"

class XOSView;

class BitMeter : public Meter {
public:
  BitMeter( XOSView *parent,  
	    const char *title = "", const char *legend ="",
	    int numBits = 1, int dolegends = 0, int dousedlegends = 0 );
  virtual ~BitMeter( void );

  void checkevent( void );
  void draw( void );
  
  void checkResources( void );
protected:
  unsigned long onColor_, offColor_;
  char *bits_, *lastbits_;
  int numbits_;

  void setBits(int startbit, unsigned char values);

  void drawBits( int manditory = 0 );
private:
};

#endif
