//
//  Copyright (c) 1999, 2006, 2015, 2016, 2018
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
    _bits.resize(n);

    for (size_t i = 0 ; i < _bits.size() ; i++)
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


void BitFieldMeter::setBits(size_t startbit, unsigned char values){
    unsigned char mask = 1;
    for (size_t i = startbit ; i < startbit + 8 ; i++){
        _bits[i] = values & mask;
        mask = mask << 1;
    }
}
