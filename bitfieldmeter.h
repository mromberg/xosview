//
//  Copyright (c) 1999, 2006, 2015, 2016, 2018 Thomas Waldmann (ThomasWaldmann@gmx.de)
//  based on work of Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef bitfieldmeter_h
#define bitfieldmeter_h

#include "fieldmeter.h"
#include "drawbits.h"


class BitFieldMeter : public FieldMeter {
public:
    BitFieldMeter(size_t numBits=1, size_t numfields=1,
      const std::string &title="", const std::string &bitlegend="",
      const std::string &fieldlegend="");

    virtual ~BitFieldMeter(void);

    // virtual from Meter
    virtual void draw(X11Graphics &g);
    virtual void drawIfNeeded(X11Graphics &g);
    virtual void checkResources(const ResDB &rdb);

protected:
    std::vector<unsigned char> _bits;
    DrawBits<unsigned char> _dbits;

    void setBits(size_t startbit, unsigned char values);
    size_t numBits(void) const { return _bits.size(); }
    void setNumBits(size_t n);

private:
    std::string _fieldLegend;
};


#endif
