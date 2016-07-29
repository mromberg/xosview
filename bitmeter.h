//
//  Copyright (c) 1994, 1995, 2006, 2015, 2016
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef BITMETER_H
#define BITMETER_H

#include "meter.h"
#include "drawbits.h"


class XOSView;


class BitMeter : public Meter {
public:
    BitMeter(const std::string &title = "", const std::string &legend ="",
      size_t numBits = 1);

    virtual ~BitMeter( void );

    // virtual from Meter
    virtual void draw(X11Graphics &g);
    virtual void drawIfNeeded(X11Graphics &g);
    virtual void checkResources(const ResDB &rdb);

    size_t numBits(void) const { return bits_.size(); }
    void setNumBits(size_t n);

protected:
    std::vector<bool> bits_;
    DrawBits<bool> _dbits;

    size_t numbits(void) const { return bits_.size(); }
    void setBits(int startbit, unsigned char values);
};


#endif
