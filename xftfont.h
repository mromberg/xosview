//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef XFT_H
#define XFT_H

#include <string>

#include <X11/Xlib.h>

struct _XftFont;
typedef _XftFont XftFont;

class X11ftFont {
public:
    X11ftFont(Display *dsp);
    X11ftFont(Display *dsp, const std::string &name);
    ~X11ftFont(void);

    bool good(void) const { return _font != 0; }
    operator bool(void) const { return good(); }

    const std::string &name(void) const { return _name; }
    XftFont *font(void) const { return _font; }

    bool setFont(const std::string &name);

    unsigned int textWidth(const std::string &str);
    unsigned int textHeight(void) const;
    int textAscent(void) const;
    int textDescent(void) const;

private:
    Display *_dsp;
    XftFont *_font;
    std::string _name;
};

#endif
