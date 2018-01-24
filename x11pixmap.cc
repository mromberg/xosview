//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "x11pixmap.h"
#include "log.h"

#ifdef HAVE_XPM
#include <X11/xpm.h>
#endif



X11Pixmap::X11Pixmap(Display *dsp, Visual *v, Drawable parent, Colormap cmap,
  unsigned long bgPixVal, int width, int height, int depth)
    : _dsp(dsp), _vis(v), _parent(parent), _cmap(cmap),
      _width(width), _height(height),
      _pmap(XCreatePixmap(dsp, _parent, _width, _height, depth)),
      _g(std::make_unique<X11Graphics>(_dsp, _vis, _pmap, false, _cmap,
          bgPixVal)) {

    logDebug << "new pixmap: " << _pmap << std::endl;
    _g->clear(0, 0, _width, _height);
}


X11Pixmap::X11Pixmap(Display *dsp, Visual *v, Drawable parent, Colormap cmap)
    : _dsp(dsp), _vis(v), _parent(parent), _cmap(cmap),
      _width(0), _height(0), _pmap(0) {
}


X11Pixmap::X11Pixmap(const X11Pixmap &rhs)
    : _dsp(0), _vis(0), _parent(0), _cmap(0),
      _width(0), _height(0), _pmap(0) {

    copy(rhs);
}


X11Pixmap::~X11Pixmap(void) {
    freeObjs();
}


X11Pixmap &X11Pixmap::operator=(const X11Pixmap &rhs) {
    if (&rhs == this)
        return *this;

    copy(rhs);

    return *this;
}


void X11Pixmap::copy(const X11Pixmap &rhs) {
    freeObjs();

    _dsp = rhs._dsp;
    _vis = rhs._vis;
    _parent = rhs._parent;
    _cmap = rhs._cmap;

    unsigned int w, h, depth;
    queryDrawable(rhs._pmap, w, h, depth);
    _width = w;
    _height = h;

    _pmap = XCreatePixmap(_dsp, _parent, _width, _height, depth);
    _g = std::make_unique<X11Graphics>(_dsp, _vis, _pmap, false, _cmap,
      rhs.g().bgPixel());

    rhs.copyTo(g(), 0, 0, _width, _height, 0, 0);
}


void X11Pixmap::freeObjs(void) {
    _g.reset();
    if (_pmap) {
        XFreePixmap(_dsp, _pmap);
        _pmap = 0;
    }
    _width = 0;
    _height = 0;
}


bool X11Pixmap::load(const std::string &fileName, bool logfail) {

    freeObjs();

#ifdef HAVE_XPM
    XpmAttributes pixmap_att;
    pixmap_att.closeness = 30000;
    pixmap_att.colormap = _cmap;
    pixmap_att.valuemask = XpmSize | XpmColormap | XpmCloseness;

    int code = XpmReadFileToPixmap(_dsp, _parent,
      const_cast<char *>(fileName.c_str()), &_pmap, NULL, &pixmap_att);

    if (code != XpmSuccess) {
        if (logfail)
            logProblem << "XpmReadFileToPixmap(" << fileName << "): "
                       << XpmGetErrorString(code) << std::endl;
        _pmap=0;
        return false;
    }

    _width = pixmap_att.width;
    _height = pixmap_att.height;
    _g = std::make_unique<X11Graphics>(_dsp, _vis, _pmap, false, _cmap, 0);
    return true;
#else
    return false;
#endif
}


void X11Pixmap::copyTo(X11Graphics &g, int src_x, int src_y,
  unsigned int width, unsigned int height, int dst_x, int dst_y) const {
    XCopyArea(g._dsp, _pmap, g._drawable, g._gc, src_x, src_y,
      width, height, dst_x, dst_y);
}

bool X11Pixmap::queryDrawable(Drawable d, unsigned int &width,
  unsigned int &height, unsigned int &depth) const {

    Window root;
    int x, y;
    unsigned int border;

    return XGetGeometry(_dsp, d, &root, &x, &y,
      &width, &height, &border, &depth);
}
