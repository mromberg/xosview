//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

//----------------------------------------------------------
//  Why Henry Ford was right...
//
 // typedef struct _XftColor {
 //                  unsigned long   pixel;
 //                  XRenderColor    color;
 //              } XftColor;
//
// typedef struct {
// 	unsigned long pixel;			/* pixel value */
// 	unsigned short red, green, blue;	/* rgb values */
// 	char flags;				/* DoRed, DoGreen, DoBlue */
// 	char pad;
// } XColor;
//
// typedef struct {
// 	    unsigned short   red;
// 	    unsigned short   green;
// 	    unsigned short   blue;
// 	    unsigned short   alpha;
// 	} XRenderColor;
//----------------------------------------------------------


#include "xftgraphics.h"
#include "log.h"

#include <X11/Xft/Xft.h>



XftGraphics::XftGraphics(Display *dsp, Drawable d, bool isWindow,
  Colormap cmap, unsigned long bgPixVal)
    : _dsp(dsp), _d(d), _isWindow(isWindow), _cmap(cmap), _bgPixVal(bgPixVal),
      _fgPixVal(bgPixVal), _font(dsp), _draw(0),
      _fgxftc(0), _bgxftc(0) {

    setBG(bgPixVal);
    setFG("white");

    if (_isWindow) {
        _draw = XftDrawCreate(_dsp, _d, DefaultVisual(_dsp,
            DefaultScreen(_dsp)), _cmap);
    }
    else {
        if (depth() == 1) {
            _draw = XftDrawCreateBitmap(_dsp, _d);
        }
        else {
            _draw = XftDrawCreateAlpha(_dsp, _d, depth());
        }
    }
}

XftGraphics::~XftGraphics(void) {
    if (_draw)
        XftDrawDestroy(_draw);

    if (_fgxftc) {
        XftColorFree(_dsp, DefaultVisual(_dsp,DefaultScreen(_dsp)),
          _cmap, _fgxftc);
        delete _fgxftc;
    }
    if (_bgxftc) {
        XftColorFree(_dsp, DefaultVisual(_dsp,DefaultScreen(_dsp)),
          _cmap, _bgxftc);
        delete _bgxftc;
    }
}

void XftGraphics::setFont(const std::string &name) {
    _font.setFont(name);
}

unsigned int XftGraphics::depth(void) {
    Window root;
    int x, y;
    unsigned int width, height, border, depth;

    XGetGeometry(_dsp, _d, &root, &x, &y,
      &width, &height, &border, &depth);

    return depth;
}

unsigned long XftGraphics::allocColor(const std::string &name) {
    XColor exact, closest;

    if (XAllocNamedColor(_dsp, _cmap, name.c_str(), &closest, &exact ) == 0) {
        logProblem << "allocColor() : failed to alloc : "
                   << name << std::endl;
        return WhitePixel(_dsp, DefaultScreen(_dsp));
    }

    return exact.pixel;
}

void XftGraphics::setFG(const std::string &color, unsigned short alpha) {
    setFG(allocColor(color), alpha);
}

void XftGraphics::setFG(unsigned long pixVal, unsigned short alpha) {
    _fgPixVal = pixVal;

    XColor def;
    def.pixel = pixVal;
    XQueryColor(_dsp, _cmap, &def);

    XRenderColor xrc;
    xrc.red = def.red;
    xrc.green = def.green;
    xrc.blue = def.blue;
    xrc.alpha = alpha;

    if (!_fgxftc)
        _fgxftc = new XftColor();
    else
        XftColorFree(_dsp, DefaultVisual(_dsp,DefaultScreen(_dsp)),
          _cmap, _fgxftc);

    XftColorAllocValue(_dsp, DefaultVisual(_dsp,DefaultScreen(_dsp)),
      _cmap, &xrc, _fgxftc);
}

void XftGraphics::setBG(const std::string &color, unsigned short alpha) {
    setBG(allocColor(color), alpha);
}

void XftGraphics::setBG(unsigned long pixVal, unsigned short alpha) {
    _bgPixVal = pixVal;

    XColor def;
    def.pixel = pixVal;
    XQueryColor(_dsp, _cmap, &def);

    XRenderColor xrc;
    xrc.red = def.red;
    xrc.green = def.green;
    xrc.blue = def.blue;
    xrc.alpha = alpha;

    if (!_bgxftc)
        _bgxftc = new XftColor();
    else
        XftColorFree(_dsp, DefaultVisual(_dsp,DefaultScreen(_dsp)),
          _cmap, _bgxftc);

    XftColorAllocValue(_dsp, DefaultVisual(_dsp,DefaultScreen(_dsp)),
      _cmap, &xrc, _bgxftc);
}

void XftGraphics::drawString(int x, int y, const std::string &str) {
    XGlyphInfo extents;
    XftTextExtents8(_dsp, _font.font(), (XftChar8 *)str.c_str(), str.size(),
      &extents);
    XClearArea(_dsp, _d, x, y-extents.height,
      extents.xOff, extents.height, False);

    //XftDrawRect(_draw, _bgxftc, x, y-extents.height, extents.xOff, extents.height);
    XftDrawString8(_draw, _fgxftc, _font.font(), x, y, (XftChar8 *)str.c_str(),
      str.size());
}
