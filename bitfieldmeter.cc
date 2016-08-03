//
//  Copyright (c) 1999, 2006, 2015, 2016
//  by Thomas Waldmann ( ThomasWaldmann@gmx.de )
//  based on work of Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "bitfieldmeter.h"


BitFieldMeter::BitFieldMeter(size_t numBits, size_t numfields,
  const std::string &title, const std::string &bitslegend,
  const std::string &fieldLegend)
    : FieldMeter(numfields, title, bitslegend),
      _bits(numBits, 0),
      _fieldLegend(fieldLegend) {
}


BitFieldMeter::~BitFieldMeter( void ){
}


void BitFieldMeter::checkResources(const ResDB &rdb){
    FieldMeter::checkResources(rdb);
    _dbits.borderColor(fgColor());
}


void BitFieldMeter::setNumBits(size_t n){
    _dbits.resize(n);
    _bits.resize(n);

    for ( unsigned int i = 0 ; i < numbits() ; i++ )
        _bits[i] = 0;
}


void BitFieldMeter::draw(X11Graphics &g) {
    _dbits.draw(_bits, g, x(), y(), width() * 2 / 6, height(), true);
    FieldMeter::draw(g);
}


void BitFieldMeter::drawIfNeeded( X11Graphics &g ) {
    _dbits.draw(_bits, g, x(), y(), width() * 2 / 6, height());
    FieldMeter::drawIfNeeded(g);
}


void BitFieldMeter::setBits(int startbit, unsigned char values){
    unsigned char mask = 1;
    for (int i = startbit ; i < startbit + 8 ; i++){
        _bits[i] = values & mask;
        mask = mask << 1;
    }
}
