//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "x11graphics.h"
#include "log.h"
#include "x11font.h"
#ifdef HAVE_XFT
#include "xftgraphics.h"
#endif

#include <iomanip>

std::vector<Pixmap>	X11Graphics::_stipples;



X11Graphics::X11Graphics(Display *dsp, Visual *v, Drawable d, bool isWindow,
  Colormap cmap, unsigned long bgPixVal)
    : _dsp(dsp), _drawable(d), _isWindow(isWindow), _cmap(cmap),
      _gc(0), _depth(0),
      _fgPixel(0), _bgPixel(bgPixVal), _width(0), _height(0), _font(0),
      _xftg(0), _doStippling(false) {
#if HAVE_XFT
    _xftg = new XftGraphics(_dsp, v, _drawable, _isWindow, _cmap, _bgPixel);
    _font = &_xftg->font();
#else
    _font = new X11Font(_dsp);
#endif
    refCount()++;
    updateInfo();
    _gc = XCreateGC(_dsp, _drawable, 0, NULL);
    // FIXME Turn this back on and fix fieldmetergraph's copyArea
    XSetGraphicsExposures(_dsp, _gc, False);
    setFont("fixed");
    setBG(_bgPixel);
    setFG("white");
    initStipples();
}


X11Graphics::~X11Graphics(void) {
    logDebug << "~X11Graphics(): " << refCount() << std::endl;

#ifdef HAVE_XFT
    delete _xftg;
#else
    delete _font;
#endif

    // The refCount is used to free global cached stipples
    refCount()--;
    releaseStipples();

    if (_gc)
        XFreeGC(_dsp, _gc);
}


void X11Graphics::clipMask(int x, int y,
  unsigned int width, unsigned int height) {

    XRectangle rectangle;

    rectangle.x = x;
    rectangle.y = y;
    rectangle.width = width;
    rectangle.height = height;

    XSetClipRectangles(_dsp, _gc, 0, 0, &rectangle, 1, YXBanded);
}


void X11Graphics::unsetClipMask(void) {
    XSetClipMask(_dsp, _gc, None);
}


void X11Graphics::updateInfo(void) {
    if (_depth && !_isWindow)
        return; // Got it already and it ain't gonna change
    else {
        // It is a window.  And may have resized
        Window root;
        int x, y;
        unsigned int border;

        XGetGeometry(_dsp, _drawable, &root, &x, &y,
          &_width, &_height, &border, &_depth);
    }
}


unsigned long X11Graphics::allocColor(Display *d, Colormap c,
  const std::string &color) {
    XColor exact, closest;

    if (XAllocNamedColor(d, c, color.c_str(),
        &closest, &exact) == 0) {
        logProblem << "X11Graphics::getPixelValue() : "
                   << "Could not allocate a pixel value for " << color
                   << std::endl;
        return WhitePixel(d, DefaultScreen(d));
    }

    // logDebug << "alloc: " << color
    //          << " -> " << closest << std::endl;

    return closest.pixel;
}


void X11Graphics::clear(int x, int y, unsigned int width, unsigned int height) {
    // XClearArea is disabled because it does
    // not work with drawing to offscreen buffers.
    // FIXME: find some way to draw a pixmap background for clearing
    if (false && _isWindow)
        XClearArea(_dsp, _drawable, x, y, width, height, False);
    else {
        XSetForeground(_dsp, _gc, _bgPixel);
        XFillRectangle(_dsp, _drawable, _gc, x, y, width, height);
        XSetForeground(_dsp, _gc, _fgPixel);
    }
}


void X11Graphics::setFG(const std::string &color) {
    unsigned long pv = 1;

    if (_depth > 1)
        pv = allocColor(color);

    setFG(pv);
}


void X11Graphics::setFG(unsigned long pixVal) {
    _fgPixel = pixVal;
    XSetForeground(_dsp, _gc, _fgPixel);
}


void X11Graphics::setBG(const std::string &color) {
    unsigned long pv = 0;

    if (_depth > 1)
        pv = allocColor(color);

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


void X11Graphics::initStipples(void) {
    if (!_stipples.size()) {
        _stipples.push_back(createPixmap(
              std::string("\000\000", 2), 2, 2));
        _stipples.push_back(createPixmap(
              std::string("\002\000\001\000", 4), 2, 4));
        _stipples.push_back(createPixmap(
              std::string("\002\001", 2), 2, 2));
        _stipples.push_back(createPixmap(
              std::string("\002\003\001\003", 4), 2, 4));
    }
}

void X11Graphics::releaseStipples(void) {
    if (!refCount()) {
        logDebug << "Free stipples..." << std::endl;
        for (size_t i = 0 ; i < _stipples.size() ; i++)
            XFreePixmap(_dsp, _stipples[i]);
        _stipples.resize(0);
    }
}

Pixmap X11Graphics::createPixmap(const std::string &data,
  unsigned int w, unsigned int h) {
    return XCreatePixmapFromBitmapData(_dsp, _drawable,
      const_cast<char *>(data.data()), w, h, 0, 1, 1);
}

void X11Graphics::setFont(const std::string &name) {
    _font->setFont(name);

#ifndef HAVE_XFT
    XGCValues gcv;
    X11Font *fnt = dynamic_cast<X11Font *>(_font);
    if (!fnt) {
        logBug << "Font is not a core X11 font object." << std::endl;
    }
    else {
        gcv.font = fnt->id();
        XChangeGC(_dsp, _gc, GCFont, &gcv);
    }
#endif
}

size_t &X11Graphics::refCount(void) {
    static size_t count = 0;
    return count;
}

void X11Graphics::drawString(int x, int y, const std::string &str) {
#ifdef HAVE_XFT
    _xftg->setFG(fgPixel());
    _xftg->drawString(x, y, str);
#else
    XDrawString(_dsp, _drawable, _gc, x, y, str.c_str(), str.size());
#endif
}


unsigned int X11Graphics::maxCharWidth(void) {
    return _font->maxCharWidth();
}


void X11Graphics::resize(unsigned int width, unsigned int height) {
    _width = width;
    _height = height;
#ifdef HAVE_XFT
    // When running DBE and the window resizes, the back buffer
    // we are drawing in has the same XID.  But Xft gets bent.  So,
    // This will reset it.
    logDebug << "Kick Xft..." << std::endl;
    _xftg->kick();
#endif
}
