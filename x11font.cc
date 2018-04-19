//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "x11font.h"
#include "log.h"



X11Font::X11Font(Display *dsp)
    : _dsp(dsp), _font(0) {
}


X11Font::X11Font(Display *dsp, const std::string &name)
    : _dsp(dsp), _name(name), _font(nullptr) {
    setFont(name);
}


X11Font::~X11Font(void) {
    if (_font)
        XFreeFont(_dsp, _font);
}


bool X11Font::setFont(const std::string &name) {
    if (_font) {
        XFreeFont(_dsp, _font);
        _font = nullptr;
        _name = "";
    }

    if ((_font = XLoadQueryFont(_dsp, name.c_str())) == nullptr)
        return false;

    _name = name;
    return true;
}


int X11Font::textAscent(void) const {
    logAssert(good()) << "can't use an uninitialized font." << std::endl;
    return _font->ascent;
}


int X11Font::textDescent(void) const {
    logAssert(good()) << "can't use an uninitialized font." << std::endl;
    return _font->descent;
}


Font X11Font::id(void) const {
    logAssert(good()) << "can't use an uninitialized font." << std::endl;
    return _font->fid;
}


unsigned int X11Font::textWidth(const std::string &str) {
    logAssert(good()) << "can't use an uninitialized font." << std::endl;
    return XTextWidth(_font, str.c_str(), str.size());
}
