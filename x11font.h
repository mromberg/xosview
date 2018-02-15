//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef x11font_h
#define x11font_h

#include "font.h"

#include <X11/Xlib.h>


class X11Font : public XOSVFont {
public:
    X11Font(Display *dsp);
    X11Font(Display *dsp, const std::string &name);
    virtual ~X11Font(void);

    virtual bool good(void) const override { return _font; }

    virtual const std::string &name(void) const override { return _name; }
    Font id(void) const;

    virtual bool setFont(const std::string &name) override;

    virtual unsigned int textWidth(const std::string &str) override;
    virtual unsigned int textHeight(void) const override;
    virtual int textAscent(void) const override;
    virtual int textDescent(void) const override;

private:
    Display *_dsp;
    std::string _name;
    XFontStruct *_font;
};



inline unsigned int X11Font::textHeight(void) const {
    return textAscent() + textDescent();
}

#endif
