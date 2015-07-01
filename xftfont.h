//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef XFTFONT_H
#define XFTFONT_H

#include "font.h"

#include <string>

#include <X11/Xlib.h>

struct _XftFont;
typedef _XftFont XftFont;

class X11ftFont : public XOSVFont {
public:
    X11ftFont(Display *dsp);
    X11ftFont(Display *dsp, const std::string &name);
    virtual ~X11ftFont(void);

    virtual bool good(void) const { return _font != 0; }

    virtual const std::string &name(void) const { return _name; }
    XftFont *font(void) const { return _font; }

    virtual bool setFont(const std::string &name);

    virtual unsigned int textWidth(const std::string &str);
    virtual unsigned int textHeight(void) const;
    virtual int textAscent(void) const;
    virtual int textDescent(void) const;

private:
    Display *_dsp;
    XftFont *_font;
    std::string _name;
};

#endif
