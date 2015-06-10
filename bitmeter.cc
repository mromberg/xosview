//
//  Copyright (c) 1994, 1995, 2006, 2015 by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: bitmeter.cc,v 1.10 2006/02/18 04:33:04 romberg Exp $
//
#include "general.h"
#include "bitmeter.h"
#include "xosview.h"

BitMeter::BitMeter( XOSView *parent,
  const std::string &title, const std::string &legend, size_t numBits,
  int docaptions, int, int dousedlegends)
  : Meter( parent, title, legend, docaptions, dousedlegends, dousedlegends ),
  disabled_(false)  {
  setNumBits(numBits);
}

BitMeter::~BitMeter( void ){
}

void BitMeter::setNumBits(size_t n){
  bits_.resize(n);
  lastbits_.resize(n);

  for ( unsigned int i = 0 ; i < numbits() ; i++ )
      bits_[i] = lastbits_[i] = 0;
}

void BitMeter::disableMeter ( void ) {
  disabled_ = true;
  onColor_ = parent_->allocColor ("gray");
  offColor_ = onColor_;
  Meter::legend ("Disabled");

}

void BitMeter::checkResources( void ){
  Meter::checkResources();
}

void BitMeter::checkevent( void ){
  drawBits();
}

void BitMeter::drawBits( int manditory ){
  static int pass = 1;

//  pass = (pass + 1) % 2;

  int x1 = x_ + 0, x2;

  for ( unsigned int i = 0 ; i < numbits() ; i++ ){
    if ( i != (numbits() - 1) )
      x2 = x_ + ((i + 1) * (width_+1)) / numbits() - 1;
    else
      x2 = x_ + (width_+1) - 1;

    if ( (bits_[i] != lastbits_[i]) || manditory ){
      if ( bits_[i] && pass )
	parent_->setForeground( onColor_ );
      else
	parent_->setForeground( offColor_ );

      parent_->drawFilledRectangle( x1, y_, x2 - x1, height_);
    }

    lastbits_[i] = bits_[i];

    x1 = x2 + 2;
  }
}

void BitMeter::draw( void ){
  parent_->lineWidth( 1 );
  parent_->setForeground( parent_->foreground() );
  parent_->drawFilledRectangle( x_ -1, y_ - 1, width_ + 2, height_ + 2 );

  parent_->lineWidth( 0 );

  if ( dolegends_ ){
    parent_->setForeground( textcolor_ );

    int offset;
    if ( dousedlegends_ )
      offset = parent_->textWidth( "XXXXXXXXX" );
    else
      offset = parent_->textWidth( "XXXXX" );

    parent_->drawString( x_ - offset + 1, y_ + height_, title_ );
    parent_->setForeground( onColor_ );
    if(docaptions_)
    {
      parent_->drawString( x_, y_ - 5, legend_ );
      }
  }

  drawBits( 1 );
}

void BitMeter::setBits(int startbit, unsigned char values){
  unsigned char mask = 1;
  for (int i = startbit ; i < startbit + 8 ; i++){
    bits_[i] = values & mask;
    mask = mask << 1;
  }
}
