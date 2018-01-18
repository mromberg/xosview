//
//  Copyright (c) 1994, 1995, 2006, 2015, 2016, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "bitmeter.h"
#include "x11graphics.h"


BitMeter::BitMeter(const std::string &title, const std::string &legend,
  size_t numBits)
    : Meter( title, legend ), _bits(numBits, 0) {
}


BitMeter::~BitMeter( void ){
}


void BitMeter::setNumBits(size_t n){
    _bits.resize(n);

    for ( unsigned int i = 0 ; i < numBits() ; i++ )
        _bits[i] = false;
}


void BitMeter::checkResources(const ResDB &rdb){
    Meter::checkResources(rdb);
    _dbits.borderColor(fgColor());
}


void BitMeter::drawIfNeeded(X11Graphics &g){
    _dbits.draw(_bits, g, x(), y(), width(), height());
}


void BitMeter::draw(X11Graphics &g){
    _dbits.draw(_bits, g, x(), y(), width(), height(), true);
    drawLabels(g);
}


void BitMeter::setBits(size_t startbit, unsigned char values){
    unsigned char mask = 1;
    for (size_t i = startbit ; i < startbit + 8 ; i++){
        _bits[i] = values & mask;
        mask = mask << 1;
    }
}
