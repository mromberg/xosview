//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "xftgraphics.h"
#include "log.h"

#include <iomanip>

#include <X11/Xft/Xft.h>

static std::ostream &operator<<(std::ostream &os, const XRenderColor &c);
static std::ostream &operator<<(std::ostream &os, const XftColor &c);


class XftImp {
public:
    XftColor *fg(bool alloc=false) {
        if (alloc) {
            _fgAlloced = true;
            return &_fgxftc;
        }
        return _fgAlloced ? &_fgxftc : nullptr;
    }
    XftColor *bg(bool alloc=false) {
        if (alloc) {
            _bgAlloced = true;
            return &_bgxftc;
        }
        return _bgAlloced ? &_bgxftc : nullptr;
    }

    XftDraw *_draw = nullptr;
    bool _fgAlloced = false;
    XftColor _fgxftc = {};
    bool _bgAlloced = false;
    XftColor _bgxftc = {};
};


XftGraphics::XftGraphics(Display *dsp, Visual *v, Drawable d, bool isWindow,
  Colormap cmap, unsigned long bgPixVal)
    : _dsp(dsp), _vis(v), _d(d), _isWindow(isWindow), _cmap(cmap),
      _bgPixVal(bgPixVal), _fgPixVal(bgPixVal), _font(dsp),
      _imp(std::make_unique<XftImp>()) {

    setBG(bgPixVal);
    setFG("white");
    logDebug << "XftGraphics: fg: " << _imp->_fgxftc
             << ", bg: " << _imp->_bgxftc << std::endl;

    if (_isWindow) {
        logDebug << "XftDrawCreate(): "
                 << std::hex << std::showbase << _d << std::endl;
        _imp->_draw = XftDrawCreate(_dsp, _d, _vis, _cmap);
    }
    else {
        if (depth() == 1) {
            logDebug << "XftDrawCreateBitmap(): "
                     << std::hex << std::showbase << _d << std::endl;
            _imp->_draw = XftDrawCreateBitmap(_dsp, _d);
        }
        else {
            logDebug << "XftDrawCreateAlpha(): "
                     << std::hex << std::showbase << _d << std::endl;
            _imp->_draw = XftDrawCreateAlpha(_dsp, _d, depth());
        }
    }
}

XftGraphics::~XftGraphics(void) {
    if (_imp->_draw)
        XftDrawDestroy(_imp->_draw);

    if (_imp->fg())
        XftColorFree(_dsp, _vis, _cmap, _imp->fg());

    if (_imp->bg())
        XftColorFree(_dsp, _vis, _cmap, _imp->bg());
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

inline static unsigned short premul(unsigned short c, unsigned short alpha) {
    unsigned int ic = c;
    ic = (ic * alpha) / 0xffff;
    return ic;
}

void XftGraphics::setFG(unsigned long pixVal, unsigned short alpha) {
    //
    // Xft (and XRender) use pre-multiplied alpha!  This is not
    // a fact that jumps out at you reading the documentation for
    // Xft (because it is not mentioned at all there).
    //
    _fgPixVal = pixVal;

    XColor def;
    def.pixel = pixVal;
    XQueryColor(_dsp, _cmap, &def);

    XRenderColor xrc;
    xrc.red = premul(def.red, alpha);
    xrc.green = premul(def.green, alpha);
    xrc.blue = premul(def.blue, alpha);
    xrc.alpha = alpha;

    if (_imp->fg())
        XftColorFree(_dsp, _vis, _cmap, _imp->fg());

    XftColorAllocValue(_dsp, _vis, _cmap, &xrc, _imp->fg(true));
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
    xrc.red = premul(def.red, alpha);
    xrc.green = premul(def.green, alpha);
    xrc.blue = premul(def.blue, alpha);
    xrc.alpha = alpha;

    if (_imp->bg())
        XftColorFree(_dsp, _vis, _cmap, _imp->bg());

    XftColorAllocValue(_dsp, _vis, _cmap, &xrc, _imp->bg(true));
}

void XftGraphics::drawString(int x, int y, const std::string &str) {
    XftDrawString8(_imp->_draw, &_imp->_fgxftc, _font.font(), x, y,
      (XftChar8 *)str.c_str(), str.size());
}


void XftGraphics::kick(void) {
    // For some unknown reason, Xft needs to be reset.
    // Setting (the same) drawable again seems to do it.
    XftDrawChange(_imp->_draw, _d);
}


//----------------------------------------------------------
// For debugging
//----------------------------------------------------------
std::ostream &operator<<(std::ostream &os, const XColor &c) {
    os << std::hex << std::showbase
       << "[ "
       << "( " << std::setw(6) << c.red
       << ", " << std::setw(6) << c.green
       << ", " << std::setw(6) << c.blue
       << " ), "
       << "flags:" << (unsigned)c.flags << ","
       << "pad:" << (unsigned)c.pad << ","
       << "pixel:" << c.pixel
       << " ]";
    os << std::dec << std::noshowbase;
    return os;
}

std::ostream &operator<<(std::ostream &os, const XRenderColor &c) {
    os << std::hex << std::showbase
       << "( " << std::setw(6) << c.red
       << ", " << std::setw(6) << c.green
       << ", " << std::setw(6) << c.blue
       << ", " << std::setw(6) << c.alpha
       << " )";
    os << std::dec << std::noshowbase;
    return os;
}

std::ostream &operator<<(std::ostream &os, const XftColor &c) {
    os << std::hex << std::showbase
       << "[ " << c.color
       << ", " << c.pixel
       << " ]";
    os << std::dec << std::noshowbase;
    return os;
}
//----------------------------------------------------------
