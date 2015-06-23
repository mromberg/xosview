//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef X11GRAPHICS_H
#define X11GRAPHICS_H

#include <string>
#include <vector>

#include <X11/Xlib.h>

class X11Graphics {
public:
    X11Graphics(Display *dsp, Drawable d, Colormap cmap,
      unsigned long bgPixVal = 0);
    ~X11Graphics(void);

    unsigned int depth(void);

    void setFG(const std::string &color);
    void setFG(unsigned long pixVal);
    void setBG(const std::string &color);
    void setBG(unsigned long pixVal);

    void clear(int x, int y, unsigned int width, unsigned int height);
    void drawRectangle(int x, int y, unsigned int width, unsigned int height);
    void drawFilledRectangle(int x, int y,
      unsigned int width, unsigned int height);
    void copyArea(int x, int y, unsigned int width, unsigned int height,
      int dest_x, int dest_y);

    //------------------------------------------------------------
    // Depricated interface
    //------------------------------------------------------------
    void setForeground(unsigned long pixVal) { setFG(pixVal); }
    void setStippleMode(bool mode) { _doStippling = mode; }
    void setStipples(const std::vector<Pixmap> &stipples);
    void setStipple(Pixmap stipple);
    void setStippleN(int n) { setStipple(_stipples[n]); }
    //---End Depricated-------------------------------------------
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

    //------------------------------------------------------------
    // Depricated interface
    //------------------------------------------------------------
    bool _doStippling;
    std::vector<Pixmap>	_stipples;	//  Array of Stipple masks.
    //---End Depricated-------------------------------------------
};

inline void X11Graphics::drawRectangle(int x, int y,
  unsigned int width, unsigned int height) {
    XDrawRectangle(_dsp, _drawable, _gc, x, y, width, height);
}

inline void X11Graphics::drawFilledRectangle(int x, int y,
  unsigned int width, unsigned int height) {
    XFillRectangle(_dsp, _drawable, _gc, x, y, width, height);
}

inline void X11Graphics::setStipple(Pixmap stipple) {
    if (!_doStippling)
        return;
    XSetStipple(_dsp, _gc, stipple);
    XGCValues xgcv;
    xgcv.fill_style = FillOpaqueStippled;
    XChangeGC (_dsp, _gc, GCFillStyle, &xgcv);
}

inline void X11Graphics::setStipples(const std::vector<Pixmap> &stipples) {
    _stipples = stipples;
}

inline void X11Graphics::copyArea(int x, int y,
  unsigned int width, unsigned int height, int dest_x, int dest_y) {
    XCopyArea(_dsp, _drawable, _drawable, _gc, x, y,
      width, height, dest_x, dest_y);
}


#endif
