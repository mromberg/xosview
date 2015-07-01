//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef X11FONT_H
#define X11FONT_H
#include "font.h"
#include "log.h"

#include <X11/Xlib.h>


class X11Font : public XOSVFont {
public:
    X11Font(Display *dsp);
    X11Font(Display *dsp, const std::string &name);
    virtual ~X11Font(void);

    bool good(void) const { return _font; }

    virtual const std::string &name(void) const { return _name; }
    Font id(void) const;

    virtual bool setFont(const std::string &name);

    virtual unsigned int textWidth(const std::string &str);
    virtual unsigned int textHeight(void) const;
    virtual int textAscent(void) const;
    virtual int textDescent(void) const;

private:
    Display *_dsp;
    std::string _name;
    XFontStruct *_font;
};

inline Font X11Font::id(void) const {
    logAssert(good()) << "can't use an uninitialized font." << std::endl;
    return _font->fid;
}

inline unsigned int X11Font::textWidth(const std::string &str) {
    logAssert(good()) << "can't use an uninitialized font." << std::endl;
    return XTextWidth(_font, str.c_str(), str.size());
}

inline int X11Font::textAscent(void) const {
    logAssert(good()) << "can't use an uninitialized font." << std::endl;
    return _font->ascent;
}

inline int X11Font::textDescent(void) const {
    logAssert(good()) << "can't use an uninitialized font." << std::endl;
    return _font->descent;
}

inline unsigned int X11Font::textHeight(void) const {
    logAssert(good()) << "can't use an uninitialized font." << std::endl;
    return textAscent() + textDescent();
}

#endif
