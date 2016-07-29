//
//  Copyright (c) 1994, 1995, 2006, 2015, 2016
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "bitmeter.h"
#include "x11graphics.h"


BitMeter::BitMeter(const std::string &title, const std::string &legend,
  size_t numBits)
    : Meter( title, legend ),
      onColor_(0), offColor_(0), bits_(numBits, 0), lastbits_(numBits, 0) {
}


BitMeter::~BitMeter( void ){
}


void BitMeter::setNumBits(size_t n){
    bits_.resize(n);
    lastbits_.resize(n);

    for ( unsigned int i = 0 ; i < numbits() ; i++ )
        bits_[i] = lastbits_[i] = 0;
}


void BitMeter::checkResources(const ResDB &rdb){
    Meter::checkResources(rdb);
}


void BitMeter::drawIfNeeded(X11Graphics &g){
    drawBits(g);
}


void BitMeter::drawBits(X11Graphics &g, bool mandatory){

    int x1 = x() + 0, x2;

    for ( unsigned int i = 0 ; i < numbits() ; i++ ){
        if ( i != (numbits() - 1) )
            x2 = x() + ((i + 1) * (width() + 1)) / numbits() - 1;
        else
            x2 = x() + (width() + 1) - 1;

        if ( (bits_[i] != lastbits_[i]) || mandatory ){
            if ( bits_[i] )
                g.setFG( onColor_ );
            else
                g.setFG( offColor_ );

            g.drawFilledRectangle( x1, y(), x2 - x1, height());
        }

        lastbits_[i] = bits_[i];

        x1 = x2 + 2;
    }
}


void BitMeter::draw(X11Graphics &g){
    g.lineWidth( 1 );
    g.setFG( fgColor() );
    g.drawFilledRectangle( x() - 1, y() - 1, width() + 2, height() + 2 );

    g.lineWidth( 0 );

    drawLabels(g);

    drawBits(g, true);
}


void BitMeter::setBits(int startbit, unsigned char values){
    unsigned char mask = 1;
    for (int i = startbit ; i < startbit + 8 ; i++){
        bits_[i] = values & mask;
        mask = mask << 1;
    }
}
