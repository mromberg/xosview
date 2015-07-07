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
    unsigned int maxCharWidth(void);
    virtual unsigned int textHeight(void) const = 0;
    virtual int textAscent(void) const = 0;
    virtual int textDescent(void) const = 0;
};

inline unsigned int XOSVFont::maxCharWidth(void) {
    unsigned int rval = 0;
    // expensive!  Loops through all printable ASCII characters
    // and returns the widest width.
    rval = 0;
    for (unsigned int c = 32 ; c < 127 ; c++)
        rval = std::max(rval, textWidth(std::string() + (char)c));

    return rval;
}

#endif
