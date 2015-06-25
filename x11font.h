//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef X11FONT_H
#define X11FONT_H

#include <string>

#include <X11/Xlib.h>


class X11Font {
public:
    X11Font(Display *dsp);
    X11Font(Display *dsp, const std::string &name);
    ~X11Font(void);

    bool good(void) const { return _font; }
    operator bool(void) const { return good(); }

    const std::string &name(void) const { return _name; }
    Font id(void) const { return _font ? _font->fid : 0; }

    bool setFont(const std::string &name);

    unsigned int textWidth(const std::string &str);
    unsigned int textHeight(void) const;
    int textAscent(void) const;
    int textDescent(void) const;

private:
    Display *_dsp;
    std::string _name;
    XFontStruct *_font;
};

inline unsigned int X11Font::textWidth(const std::string &str) {
    return XTextWidth(_font, str.c_str(), str.size());
}

inline int X11Font::textAscent(void) const {
    return _font->ascent;
}

inline int X11Font::textDescent(void) const {
    return _font->descent;
}

inline unsigned int X11Font::textHeight(void) const {
    return textAscent() + textDescent();
}

#endif
