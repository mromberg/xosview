//
//  Copyright (c) 2015
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
    : _dsp(dsp), _name(name), _font(0) {
    setFont(name);
}

X11Font::~X11Font(void) {
    if (_font)
        XFreeFont(_dsp, _font);
}

bool X11Font::setFont(const std::string &name) {
    if (_font) {
        XFreeFont(_dsp, _font);
        _font = 0;
        _name = "";
    }

    if ((_font = XLoadQueryFont(_dsp, name.c_str())) == NULL)
        return false;

    _name = name;
    return true;
}
