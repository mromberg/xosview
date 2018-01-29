//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef x11graphics_h
#define x11graphics_h

#include "font.h"

#include <vector>
#include <memory>

#include <X11/Xlib.h>

class X11Pixmap;
class XftGraphics;
class X11Font;


class X11Graphics {
public:
    X11Graphics(Display *dsp, Visual *v, Drawable d, bool isWindow,
      Colormap cmap, unsigned long bgPixVal);
    ~X11Graphics(void);

    // Inform the Graphics object that the drawable (window)
    // bas been resized.  The call with the new width and height
    // is faster.
    void resize(void);
    void resize(unsigned int width, unsigned int height);

    unsigned int depth(void);
    unsigned int width(void) const { return _width; }
    unsigned int height(void) const { return _height; }
    unsigned int textWidth(const std::string &str);
    unsigned int maxCharWidth(void);  // expensive!
    unsigned int textHeight(void) const;
    int textAscent(void) const;
    int textDescent(void) const;

    void flush(void) { XFlush(_dsp); }

    static unsigned long allocColor(Display *d, Colormap c,
      const std::string &color);
    unsigned long allocColor(const std::string &name)
        { return allocColor(_dsp, _cmap, name); }
    void setFG(const std::string &color);
    void setFG(unsigned long pixVal);
    void setBG(const std::string &color);
    void setBG(unsigned long pixVal);
    void setBG(const X11Pixmap &pmap);
    unsigned long fgPixel(void) const { return _fgPixel; }
    unsigned long bgPixel(void) const { return _bgPixel; }

    void lineWidth(unsigned int width);
    void setFont(const std::string &name);

    void clear(int x, int y, unsigned int width, unsigned int height);
    void clear(void) { clear(0, 0, _width, _height); }

    void clipMask(int x, int y, unsigned int width, unsigned int height);
    void unsetClipMask(void);

    void drawString(int x, int y, const std::string &str);
    void drawRectangle(int x, int y, unsigned int width, unsigned int height);
    void drawFilledRectangle(int x, int y,
      unsigned int width, unsigned int height);
    void drawLine(int x1, int y1, int x2, int y2)
        { XDrawLine(_dsp, _drawable, _gc, x1, y1, x2, y2); }
    void copyArea(int x, int y, unsigned int width, unsigned int height,
      int dest_x, int dest_y);

    std::unique_ptr<X11Pixmap> newX11Pixmap(unsigned int width,
      unsigned int height);

    //------------------------------------------------------------
    // Depricated interface
    //------------------------------------------------------------
    void setForeground(unsigned long pixVal) { setFG(pixVal); }
    void setBackground(unsigned long pixVal) { setBG(pixVal); }
    void setStippleMode(bool mode) { _doStippling = mode; }
    void setStipple(Pixmap stipple);
    void setStippleN(unsigned int n) { setStipple(_stipples[n]); }
    Pixmap createPixmap(const std::string &data,
      unsigned int w, unsigned int h);
    //---End Depricated-------------------------------------------
private:
    Display *_dsp;
    Drawable _drawable;
    bool _isWindow;  // because XClearArea only works with 'em (no way to tell)
    Colormap _cmap;
    GC _gc;
    unsigned int _depth;
    unsigned long _fgPixel;
    unsigned long _bgPixel;
    std::unique_ptr<X11Pixmap> _bgPixmap;
    unsigned int _width;
    unsigned int _height;
    std::unique_ptr<XftGraphics> _xftg;
    std::unique_ptr<X11Font> _x11font;
    Visual *_visual;

    XOSVFont *font(void) const;
    void updateInfo(void); // update _width, _height, _depth

    //------------------------------------------------------------
    // Depricated interface
    //------------------------------------------------------------
    bool _doStippling;
    void initStipples(void);
    void releaseStipples(void);
    static std::vector<Pixmap>	_stipples;	//  Array of Stipple masks.
    static size_t &refCount(void);
    //---End Depricated-------------------------------------------

    friend class X11Pixmap;
};

inline void X11Graphics::drawRectangle(int x, int y,
  unsigned int width, unsigned int height) {
    XDrawRectangle(_dsp, _drawable, _gc, x, y, width, height);
}

inline void X11Graphics::drawFilledRectangle(int x, int y,
  unsigned int width, unsigned int height) {
    // Add one to match XDrawRectangle()  the fill width/height is one short
    // by design.  The design is stupid.
    XFillRectangle(_dsp, _drawable, _gc, x, y, width+1, height+1);
}

inline void X11Graphics::setStipple(Pixmap stipple) {
    if (!_doStippling)
        return;
    XSetStipple(_dsp, _gc, stipple);
    XGCValues xgcv;
    xgcv.fill_style = FillOpaqueStippled;
    XChangeGC (_dsp, _gc, GCFillStyle, &xgcv);
}

inline void X11Graphics::copyArea(int x, int y,
  unsigned int width, unsigned int height, int dest_x, int dest_y) {
    XCopyArea(_dsp, _drawable, _drawable, _gc, x, y,
      width, height, dest_x, dest_y);
}

inline void X11Graphics::resize(void) {
    updateInfo();
}


inline void X11Graphics::lineWidth(unsigned int width) {
    XGCValues xgcv;
    xgcv.line_width = width;
    XChangeGC(_dsp, _gc, GCLineWidth, &xgcv);
}

inline unsigned int X11Graphics::textWidth(const std::string &str) {
    return font()->textWidth(str);
}

inline int X11Graphics::textAscent(void) const {
    return font()->textAscent();
}

inline int X11Graphics::textDescent(void) const {
    return font()->textDescent();
}

inline unsigned int X11Graphics::textHeight(void) const {
    return textAscent() + textDescent();
}

#endif
