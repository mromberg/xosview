//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "x11graphics.h"
#include "log.h"

X11Graphics::X11Graphics(Display *dsp, Drawable d, Colormap cmap,
  unsigned long bgPixVal)
    : _dsp(dsp), _drawable(d), _cmap(cmap), _gc(0), _depth(0),
      _fgPixel(0), _bgPixel(bgPixVal), _doStippling(false) {

    _gc = XCreateGC(_dsp, _drawable, 0, NULL);
    _depth = getDepth(_dsp, _drawable);
    setBG(_bgPixel);
    setFG("white");
}

X11Graphics::~X11Graphics(void) {
    XFreeGC(_dsp, _gc);
}

unsigned int X11Graphics::getDepth(Display *dsp, Drawable d) const {
    Window root;
    int x, y;
    unsigned int width, height, border, depth;

    XGetGeometry(dsp, d, &root, &x, &y,
      &width, &height, &border, &depth);

    return depth;
}

unsigned long X11Graphics::getPixelValue(const std::string &color) const {
    // unsigned long val = 0;
    // if (_pixelCache.map(color, val))
    //     return val;

    XColor exact, closest;

    if (XAllocNamedColor(_dsp, _cmap, color.c_str(),
        &closest, &exact) == 0) {
        logProblem << "X11Graphics::getPixelValue() : "
                   << "Could not allocate a pixel value for " << color
                   << std::endl;
        return WhitePixel(_dsp, DefaultScreen(_dsp));
    }

    //_pixelCache.add(color, closest.pixel);

    return closest.pixel;
}

void X11Graphics::clear(int x, int y, unsigned int width, unsigned int height) {
    XSetForeground(_dsp, _gc, _bgPixel);
    XFillRectangle(_dsp, _drawable, _gc, x, y, width, height);
    XSetForeground(_dsp, _gc, _fgPixel);
}

void X11Graphics::setFG(const std::string &color) {
    unsigned long pv = 1;

    if (_depth > 1)
        pv = getPixelValue(color);

    setFG(pv);
}

void X11Graphics::setFG(unsigned long pixVal) {
    _fgPixel = pixVal;
    XSetForeground(_dsp, _gc, _fgPixel);
}

void X11Graphics::setBG(const std::string &color) {
    unsigned long pv = 0;

    if (_depth > 1)
        pv = getPixelValue(color);

    setBG(pv);
}

void X11Graphics::setBG(unsigned long pixVal) {
    _bgPixel = pixVal;
    XSetBackground(_dsp, _gc, _bgPixel);
}

unsigned int X11Graphics::depth(void) {
    Window root;
    int x, y;
    unsigned int width, height, border, depth;

    XGetGeometry(_dsp, _drawable, &root, &x, &y,
      &width, &height, &border, &depth);

    return depth;
}
