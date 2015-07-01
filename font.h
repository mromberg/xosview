//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef FONT_H
#define FONT_H

#include <string>

class XOSVFont {
public:
    virtual ~XOSVFont(void) {}

    virtual bool good(void) const = 0;
    operator bool(void) const { return good(); }

    virtual const std::string &name(void) const = 0;

    virtual bool setFont(const std::string &name) = 0;

    virtual unsigned int textWidth(const std::string &str) = 0;
    virtual unsigned int textHeight(void) const = 0;
    virtual int textAscent(void) const = 0;
    virtual int textDescent(void) const = 0;
};

#endif
