//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef X11GRAPHICS_H
#define X11GRAPHICS_H

#include <string>

#include <X11/Xlib.h>

class X11Graphics {
public:
    X11Graphics(Display *dsp, Drawable d, Colormap cmap,
      const std::string &bgColor);
    ~X11Graphics(void);

    void clear(int x, int y, unsigned int width, unsigned int height);

    void setFG(const std::string &color);
    void setFG(unsigned long pixVal);

    void setBG(const std::string &color);
    void setBG(unsigned long pixVal);

    void drawRectangle(int x, int y, unsigned int width, unsigned int height);

private:
    Display *_dsp;
    Drawable _drawable;
    Colormap _cmap;
    GC _gc;
    unsigned int _depth;
    unsigned long _fgPixel;
    unsigned long _bgPixel;

    unsigned int getDepth(Display *dsp, Drawable d) const;
    unsigned long getPixelValue(const std::string &color) const;
};

inline void X11Graphics::drawRectangle(int x, int y,
  unsigned int width, unsigned int height) {
    XDrawRectangle(_dsp, _drawable, _gc, x, y, width, height);
}

#endif
