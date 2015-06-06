//
//  Copyright (c) 1994, 1995, 2006, 2015 by Mike Romberg ( mike.romberg@noaa.gov )
//
//  This file may be distributed under terms of the GPL
//
#ifndef _BITMETER_H_
#define _BITMETER_H_

#include "meter.h"
#include <string>

class XOSView;

class BitMeter : public Meter {
public:
  BitMeter( XOSView *parent,
    const std::string &title = "", const std::string &legend ="",
    int numBits = 1, int docaptions = 0, int dolegends = 0,
    int dousedlegends = 0 );
  virtual ~BitMeter( void );

  void checkevent( void );
  void draw( void );
  void disableMeter ( void );

  int numBits(void) const { return numbits_; }
  void setNumBits(int n);

  void checkResources( void );
protected:
  unsigned long onColor_, offColor_;
  char *bits_, *lastbits_;
  int numbits_;
  bool disabled_;

  void setBits(int startbit, unsigned char values);

  void drawBits( int manditory = 0 );
private:
};

#endif
