//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef X11PIXMAP_H
#define X11PIXMAP_H

#include "x11graphics.h"

#include <string>

#include <X11/Xlib.h>

class X11Pixmap {
public:
    X11Pixmap(Display *dsp, Visual *v, Drawable parent, Colormap cmap,
      unsigned long bgPixVal, int width, int height, int depth);
    X11Pixmap(Display *dsp, Visual *v, Drawable parent, Colormap cmap);
    X11Pixmap(const X11Pixmap &rhs);

    ~X11Pixmap(void);

    X11Pixmap &operator=(const X11Pixmap &rhs);

    Pixmap pmap(void) const { return _pmap; }
    X11Graphics &g(void) const { return *_g; }

    int width(void) const { return _width; }
    int height(void) const { return _height; }

    void copyTo(X11Graphics &g, int src_x, int src_y,
      unsigned int width, unsigned int height, int dst_x, int dst_y) const;
    void copyTo(X11Graphics &g, int dst_x, int dst_y) const
        { copyTo(g, 0, 0, _width, _height, dst_x, dst_y); }

    bool load(const std::string &fileName, bool logfail=true);

private:
    Pixmap _pmap;
    Display *_dsp;
    Visual *_vis;
    Drawable _parent;
    Colormap _cmap;
    X11Graphics *_g;
    int _width;
    int _height;

    void freeObjs(void);
    bool queryDrawable(Drawable d, unsigned int &width, unsigned int &height,
      unsigned int &depth) const;
    void copy(const X11Pixmap &rhs);
};


#endif
