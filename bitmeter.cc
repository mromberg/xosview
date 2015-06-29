//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
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
    onColor_ = parent_->g().allocColor ("gray");
    offColor_ = onColor_;
    Meter::legend ("Disabled");
}

void BitMeter::checkResources( void ){
    Meter::checkResources();
}

void BitMeter::checkevent( void ){
    drawBits(parent_->g());
}

void BitMeter::drawBits(X11Graphics &g, bool manditory){
    static int pass = 1;

    int x1 = x_ + 0, x2;

    for ( unsigned int i = 0 ; i < numbits() ; i++ ){
        if ( i != (numbits() - 1) )
            x2 = x_ + ((i + 1) * (width_+1)) / numbits() - 1;
        else
            x2 = x_ + (width_+1) - 1;

        if ( (bits_[i] != lastbits_[i]) || manditory ){
            if ( bits_[i] && pass )
                g.setFG( onColor_ );
            else
                g.setFG( offColor_ );

            g.drawFilledRectangle( x1, y_, x2 - x1, height_);
        }

        lastbits_[i] = bits_[i];

        x1 = x2 + 2;
    }
}

void BitMeter::draw(X11Graphics &g){
    g.lineWidth( 1 );
    g.setFG( parent_->foreground() );
    g.drawFilledRectangle( x_ -1, y_ - 1, width_ + 2, height_ + 2 );

    g.lineWidth( 0 );

    if ( dolegends_ ){
        g.setFG( textcolor_ );

        int offset;
        if ( dousedlegends_ )
            offset = g.textWidth( "XXXXXXXXX" );
        else
            offset = g.textWidth( "XXXXX" );

        g.drawString( x_ - offset + 1, y_ + height_, title_ );
        g.setFG( onColor_ );
        if(docaptions_) {
            g.drawString( x_, y_ - 5, legend_ );
        }
    }

    drawBits(g, true);
}

void BitMeter::setBits(int startbit, unsigned char values){
    unsigned char mask = 1;
    for (int i = startbit ; i < startbit + 8 ; i++){
        bits_[i] = values & mask;
        mask = mask << 1;
    }
}
