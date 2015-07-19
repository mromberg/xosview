//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "x11pixmap.h"
#include "log.h"

X11Pixmap::X11Pixmap(Display *dsp, Drawable parent, Colormap cmap,
  unsigned long bgPixVal, int width, int height, int depth)
    : _pmap(0), _dsp(dsp), _parent(parent), _g(0),
      _width(width), _height(height) {

    _pmap = XCreatePixmap(dsp, _parent, _width, _height, depth);
    logDebug << "new pixmap: " << _pmap << std::endl;
    _g = new X11Graphics(_dsp, _pmap, false, cmap, bgPixVal);

    _g->clear(0, 0, _width, _height);
}

X11Pixmap::~X11Pixmap(void) {
    delete _g;
    XFreePixmap(_dsp, _pmap);
}

void X11Pixmap::copyTo(X11Graphics &g, int src_x, int src_y,
  unsigned int width, unsigned int height, int dst_x, int dst_y) {
    XCopyArea(g._dsp, _pmap, g._drawable, g._gc, src_x, src_y,
      width, height, dst_x, dst_y);
}
