//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef xftfont_h
#define xftfont_h

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

    X11ftFont(X11ftFont &) = delete;
    X11ftFont &operator=(const X11ftFont &) = delete;

    virtual bool good(void) const override { return _font != nullptr; }

    virtual const std::string &name(void) const override { return _name; }
    XftFont *font(void) const { return _font; }

    virtual bool setFont(const std::string &name) override;

    virtual unsigned int textWidth(const std::string &str) override;
    virtual unsigned int textHeight(void) const override;
    virtual int textAscent(void) const override;
    virtual int textDescent(void) const override;

private:
    Display *_dsp;
    XftFont *_font;
    std::string _name;
};

#endif
