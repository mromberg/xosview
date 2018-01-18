//
//  Copyright (c) 1994, 1995, 2006, 2015, 2016, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef bitmeter_h
#define bitmeter_h

#include "meter.h"
#include "drawbits.h"


class BitMeter : public Meter {
public:
    BitMeter(const std::string &title="", const std::string &legend="",
      size_t numBits=1);

    virtual ~BitMeter(void);

    // virtual from Meter
    virtual void draw(X11Graphics &g);
    virtual void drawIfNeeded(X11Graphics &g);
    virtual void checkResources(const ResDB &rdb);

protected:
    std::vector<bool> _bits;
    DrawBits<bool> _dbits;

    size_t numBits(void) const { return _bits.size(); }
    void setBits(size_t startbit, unsigned char values);
    void setNumBits(size_t n);
};


#endif
