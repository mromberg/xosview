//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef xftgraphics_h
#define xftgraphics_h

#include "xftfont.h"

#include <memory>

#include <X11/Xlib.h>

class XftImp;



class XftGraphics {
public:
    XftGraphics(Display *dsp, Visual *v, Drawable d, bool isWindow,
      Colormap cmap, unsigned long bgPixVal);
    ~XftGraphics(void);

    void setFont(const std::string &name);
    unsigned int depth(void);

    unsigned long allocColor(const std::string &name);
    void setFG(const std::string &color, unsigned short alpha=0xffff);
    void setFG(unsigned long pixVal, unsigned short alpha=0xffff);
    void setBG(const std::string &color, unsigned short alpha=0xffff);
    void setBG(unsigned long pixVal, unsigned short alpha=0xffff);
    unsigned long fgPixel(void) const { return _fgPixVal; }
    unsigned long bgPixel(void) const { return _bgPixVal; }

    unsigned int textWidth(const std::string &str);
    unsigned int textHeight(void) const;
    int textAscent(void) const;
    int textDescent(void) const;

    void drawString(int x, int y, const std::string &str);

    X11ftFont &font(void) { return _font; }
    void kick(void);

private:
    Display *_dsp;
    Visual *_vis;
    Drawable _d;
    bool _isWindow;
    Colormap _cmap;
    unsigned long _bgPixVal;
    unsigned long _fgPixVal;
    X11ftFont _font;
    std::unique_ptr<XftImp> _imp;
};

// color printing operators.
std::ostream &operator<<(std::ostream &os, const XColor &c);


#endif
