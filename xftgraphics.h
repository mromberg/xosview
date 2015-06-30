//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef XFTGRAPHICS_H
#define XFTGRAPHICS_H

#include "xftfont.h"

#include <X11/Xlib.h>

struct _XftDraw;
typedef _XftDraw XftDraw;

struct _XftColor;
typedef _XftColor XftColor;



class XftGraphics {
public:
    XftGraphics(Display *dsp, Drawable d, bool isWindow, Colormap cmap,
      unsigned long bgPixVal);
    ~XftGraphics(void);

    void setFont(const std::string &name);
    unsigned int depth(void);

    unsigned long allocColor(const std::string &name);
    void setFG(const std::string &color, unsigned short alpha=0);
    void setFG(unsigned long pixVal, unsigned short alpha=0);
    void setBG(const std::string &color, unsigned short alpha=0xffff);
    void setBG(unsigned long pixVal, unsigned short alpha=0xffff);
    unsigned long fgPixel(void) const { return _fgPixVal; }
    unsigned long bgPixel(void) const { return _bgPixVal; }

    unsigned int textWidth(const std::string &str);
    unsigned int textHeight(void) const;
    int textAscent(void) const;
    int textDescent(void) const;

    void drawString(int x, int y, const std::string &str);

private:
    Display *_dsp;
    Drawable _d;
    bool _isWindow;
    Colormap _cmap;
    unsigned long _bgPixVal;
    unsigned long _fgPixVal;
    X11ftFont _font;
    XftDraw *_draw;
    XftColor *_fgxftc;
    XftColor *_bgxftc;
};

#endif
