//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

//---------------------------------------------------
//
// "sans-7:spacing=mono"
// "morpheus-18"
//
//---------------------------------------------------
#include "xftfont.h"
#include "log.h"

#include <X11/Xft/Xft.h>


X11ftFont::X11ftFont(Display *dsp, const std::string &name)
    : _dsp(dsp), _font(nullptr), _name(name) {
    setFont(name);
}


X11ftFont::X11ftFont(Display *dsp): _dsp(dsp), _font(nullptr) {
    // std::string text("Hello World");
    // XGlyphInfo extents;
    // XftTextExtents8(_dsp, font, (XftChar8 *)text.c_str(), text.size(),
    //   &extents);
    // //int ytext = extents.height - extents.y;
    // // xOff is where the next glyph/text would occur
    // logEvent << "height: " << extents.height << std::endl;
    // logEvent << "width: " << extents.xOff << std::endl;
}


X11ftFont::~X11ftFont(void) {
    // According to the man page:
    // XftFontClose() is used to mark an XftFont as unused.
    // XftFonts  are internally  allocated,  reference-counted,
    // and freed by Xft; the programmer does not ordinarily need
    // to allocate or free  storage

    // However calling this fixes a valgrind memory leak.
    // The Xemacs gang seems to be calling XftFontClose()
    // So I will too (what could go wrong?...)
    if (_font)
        XftFontClose(_dsp, _font);
}


bool X11ftFont::setFont(const std::string &name) {
    if (_font)
        XftFontClose(_dsp, _font);
    _font = XftFontOpenName(_dsp, DefaultScreen(_dsp), name.c_str());
    return good();
}


unsigned int X11ftFont::textWidth(const std::string &str) {
    if (good()) {
        XGlyphInfo extents;
        XftTextExtents8(_dsp, _font, (XftChar8 *)str.c_str(), str.size(),
          &extents);
        return extents.xOff;
    }

    logBug << "textWidth() of bad font: " << name() << std::endl;
    return 0;
}


unsigned int X11ftFont::textHeight(void) const {
    if (good()) {
        XGlyphInfo extents;
        XftTextExtents8(_dsp, _font, (XftChar8 *)"Ag", 2,
          &extents);
        return extents.height;
    }

    logBug << "textHeight() of bad font: " << name() << std::endl;
    return 0;
}


int X11ftFont::textAscent(void) const {
    if (good())
        return _font->ascent;

    logBug << "textAscent() of bad font: " << name() << std::endl;
    return 0;
}


int X11ftFont::textDescent(void) const {
    if (good())
        return _font->descent;

    logBug << "textDescent() of bad font: " << name() << std::endl;
    return 0;
}
