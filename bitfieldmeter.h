//
//  Copyright (c) 1999, 2006, 2015, 2016 Thomas Waldmann (ThomasWaldmann@gmx.de)
//  based on work of Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef BITFIELDMETER_H
#define BITFIELDMETER_H

#include "fieldmeter.h"
#include "drawbits.h"


class BitFieldMeter : public FieldMeter {
public:
    BitFieldMeter( size_t numBits=1, size_t numfields=1,
      const std::string &title = "", const std::string &bitlegend = "",
      const std::string &fieldlegend = "");

    virtual ~BitFieldMeter( void );

    // virtual from Meter
    virtual void draw(X11Graphics &g);
    virtual void drawIfNeeded(X11Graphics &g);
    virtual void checkResources(const ResDB &rdb);

protected:
    std::vector<char> _bits;
    DrawBits<char> _dbits;

    void setBits(int startbit, unsigned char values);
    unsigned int numbits(void) const { return _bits.size(); }
    void setNumBits(int n);

    // Values used to draw the fields only.
    virtual int fldx(void) const { return x() + width() * 2 / 6 + 4; }
    virtual int fldwidth(void) const { return width() * 2 / 3 - 4; }

private:
    std::string _fieldLegend;
};


#endif
