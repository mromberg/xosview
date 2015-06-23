//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "x11pixmap.h"

X11Pixmap::X11Pixmap(Display *dsp, Drawable parent, Colormap cmap,
  unsigned long bgPixVal, int width, int height, int depth)
    : _pmap(0), _dsp(dsp), _parent(parent), _g(0),
      _width(width), _height(height) {

    _pmap = XCreatePixmap(dsp, parent, _width, _height, depth);
    _g = new X11Graphics(_dsp, _pmap, cmap, bgPixVal);

    _g->clear(0, 0, _width, _height);
}

X11Pixmap::~X11Pixmap(void) {
    delete _g;
    XFreePixmap(_dsp, _pmap);
}
