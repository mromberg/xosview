//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef BITMETER_H
#define BITMETER_H

#include "meter.h"

#include <string>
#include <vector>

class XOSView;

class BitMeter : public Meter {
public:
    BitMeter( XOSView *parent,
      const std::string &title = "", const std::string &legend ="",
      size_t numBits = 1);

    virtual ~BitMeter( void );

    // virtual from Meter
    virtual void checkevent( void );
    virtual void draw(X11Graphics &g);
    virtual void checkResources(const ResDB &rdb);

    void disableMeter ( void );

    size_t numBits(void) const { return bits_.size(); }
    void setNumBits(size_t n);

protected:
    unsigned long onColor_, offColor_;
    std::vector<char> bits_;
    std::vector<char> lastbits_;
    size_t numbits(void) const { return bits_.size(); }
    bool disabled_;

    void setBits(int startbit, unsigned char values);

    void drawBits(X11Graphics &g, bool mandatory=false);
private:
};

#endif
