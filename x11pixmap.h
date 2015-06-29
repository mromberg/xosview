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
    X11Pixmap(Display *dsp, Drawable parent, Colormap cmap,
      unsigned long bgPixVal, int width, int height, int depth);
    ~X11Pixmap(void);

    X11Graphics &g(void) { return *_g; }

    int width(void) const { return _width; }
    int height(void) const { return _height; }

    void copyTo(X11Graphics &g, int src_x, int src_y,
      unsigned int width, unsigned int height, int dst_x, int dst_y);
    void copyTo(X11Graphics &g, int dst_x, int dst_y)
        { copyTo(g, 0, 0, _width, _height, dst_x, dst_y); }

private:
    Pixmap _pmap;
    Display *_dsp;
    Drawable _parent;
    X11Graphics *_g;
    int _width;
    int _height;
};


#endif