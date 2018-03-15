//
//  Copyright (c) 2015, 2016, 2017, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "xwin.h"
#include "Xrm.h"
#include "log.h"
#include "x11pixmap.h"
#include "strutil.h"
#include "fsutil.h"

#include <X11/Xatom.h>
#ifdef HAVE_DBE
#include <X11/extensions/Xdbe.h>
#endif


static std::ostream &operator<<(std::ostream &os, const XEvent &e);


XWin::XWin()
    : _done(false),
      _wm(None), _wmdelete(None), _x(0), _y(0), _width(1), _height(1),
      _visual(nullptr), _display(nullptr), _window(0), _fgColor(0), _bgColor(0),
      _colormap(0), _transparent(false), _dbe(false), _bb(0) {
}


XWin::~XWin(void) {
    // remove the Graphics interface before the display is closed.
    _graphics.reset();

#ifdef HAVE_DBE
    if (_dbe && _bb)
        XdbeDeallocateBackBufferName(display(), _bb);
#endif

    if (display()) {
        XDestroyWindow(display(), window());

        if (XVisualIDFromVisual(visual())
          != XVisualIDFromVisual(DefaultVisual(display(), screen())))
            XFreeColormap(display(), colormap());

        // close the connection to the display
        XCloseDisplay(display());
    }
}


void XWin::openDisplay(void) {
    // Open connection to display selected by user
    const char *dname = displayName().empty() ? nullptr : displayName().c_str();
    if ((_display = XOpenDisplay(dname)) == nullptr) {
        logFatal << "Can't open display named "
                 << "'" << (dname == nullptr ? "NULL" : dname) << "'"
                 << std::endl;
    }
}


void XWin::createWindow(void) {

    setColors();

    XSizeHints *szHints = getGeometry();

    XVisualInfo tmplt;
    tmplt.visualid = XVisualIDFromVisual(visual());
    int ninfo = 0;
    XVisualInfo *vinfo = XGetVisualInfo(display(), VisualIDMask, &tmplt,
      &ninfo);
    if (ninfo != 1)
        logFatal << "Failed to locate XVisualInfo." << std::endl;

    // Define the windows attributes
    XSetWindowAttributes attr;
    unsigned long amask = 0;
    attr.colormap = colormap();           amask |= CWColormap;
    attr.border_pixel = background();     amask |= CWBorderPixel;
    attr.background_pixel = background(); amask |= CWBackPixel;
    attr.backing_store = WhenMapped;      amask |= CWBackingStore;
    attr.bit_gravity = ForgetGravity;     amask |= CWBitGravity;

    _window = XCreateWindow(display(), DefaultRootWindow(display()),
      szHints->x, szHints->y, szHints->width, szHints->height,
      0, vinfo->depth, InputOutput, visual(), amask, &attr);

    XFree(vinfo);

#ifdef HAVE_DBE
    if (_dbe) {
        _bb = XdbeAllocateBackBufferName(display(), window(), XdbeCopied);
        logDebug << "BACK BUFFER ID: " << std::hex << std::showbase
                 << _bb << std::endl;
    }
    else
        _bb = window();
#else
    _bb = window();
#endif

    setHints(szHints);
    XFree(szHints);
    szHints = nullptr;

    // Create Graphics interface.
    _graphics = std::make_unique<X11Graphics>(display(), visual(), _bb, true,
      colormap(), background());
    g().setFont(resdb().getResource("font"));
    g().setBG(background());
    g().setFG(foreground());
    g().setStippleMode(resdb().isResourceTrue("enableStipple"));

    // Pixmap backgrounds
    std::string pixmapFName = resdb().getResourceOrUseDefault(
        "pixmapName", "");
    X11Pixmap x11p(display(), visual(), window(), colormap());
    if (pixmapFName.size() && x11p.load(pixmapFName))
        g().setBG(x11p);

    // add the events
    for (const auto &event : _events)
        selectEvents(event.mask());

    // Map the main window
    map();
}


Visual *XWin::getVisual(void) {

    Visual *rval = DefaultVisual(display(), screen());

    if(resdb().isResourceTrue("transparent")) {
        XVisualInfo vinfo;
        if (XMatchVisualInfo(display(), DefaultScreen(display()),
            32, TrueColor, &vinfo)) {
            logDebug << "Found 32 bit ARGB visual: "
                     << std::hex << std::showbase
                     << vinfo.visualid << std::endl;
            _transparent = true;
            rval = vinfo.visual;
        }
        else
            logProblem << "transparent set but no 32bit ARGB visual.\n";
    }

    _dbe = isDBE(rval);
    if (_dbe)
        logDebug << "Enabling DBE..." << std::endl;

    return rval;
}


bool XWin::isDBE(Visual *v) const {
    bool rval = false;

#ifdef HAVE_DBE
    VisualID visID = XVisualIDFromVisual(v);

    int major, minor;
    if (XdbeQueryExtension(display(), &major, &minor)) {
        logDebug << "HAVE DBE: " << major << "." << minor << std::endl;

        Window rootWindow = XDefaultRootWindow(display());
        int numsp = 1;
        XdbeScreenVisualInfo *dbeVisuals = XdbeGetVisualInfo(display(),
          &rootWindow, &numsp);

        if (dbeVisuals != nullptr) {
            for (int i = 0 ; i < dbeVisuals->count ; i++) {
                if (visID == dbeVisuals->visinfo[i].visual) {
                    rval = true;
                    break;
                }
            }
            XdbeFreeVisualInfo(dbeVisuals);
        }
    }
#else
    (void) v; // avoid the warning
#endif

    return rval;
}


void XWin::setEvents(void) {
    // Set up the default Events
    addEvent(ConfigureNotify, [this](auto e){ configureEvent(e); });
    addEvent(ClientMessage, [this](auto e){ deleteEvent(e); });
    addEvent(MappingNotify, [this](auto e){ mappingNotify(e); });
}


void XWin::swapBB(void) const {
#ifdef HAVE_DBE
    if (_dbe) {
        XdbeSwapInfo swinfo;
        swinfo.swap_window = window();
        swinfo.swap_action = XdbeCopied;
        XdbeSwapBuffers(display(), &swinfo, 1);
    }
#endif
}


void XWin::setHints(XSizeHints *szHints) {
    // Set up the window manager hints
    XWMHints      *wmhints;      //  Hints for the window manager
    if ((wmhints = XAllocWMHints()) == nullptr) {
        logFatal << "Error allocating Window Manager hints!" << std::endl;
    }
    wmhints->flags = InputHint | StateHint;
    wmhints->input = True;
    wmhints->initial_state = resdb().isResourceTrue("iconic") ? IconicState
        : NormalState;

    // Set up XTextProperty for window name and icon name
    char *np = const_cast<char *>(_name.c_str());
    XTextProperty titlep;
    if (XStringListToTextProperty(&np, 1, &titlep) == 0) {
        logFatal << "Error creating XTextProperty!" << std::endl;
    }
    XTextProperty iconnamep;
    if (XStringListToTextProperty(&np, 1, &iconnamep) == 0) {
        logFatal << "Error creating XTextProperty!" << std::endl;
    }

    // Set up class hint for window manager.
    XClassHint *classhints = XAllocClassHint();
    if(classhints == nullptr)
        logFatal << "Error allocating class hint!" << std::endl;
    const std::string cname = resdb().className();
    const std::string iname = resdb().instanceName();
    classhints->res_name = const_cast<char *>(iname.c_str());
    classhints->res_class = const_cast<char *>(cname.c_str());

    if (!resdb().getResource("sessionID").empty()) {
        // X11R6 Session Manager gave us an ID.  Command handled elsewhere.
        XSetWMProperties(display(), window(), &titlep, &iconnamep, nullptr,
          0, szHints, wmhints, classhints);
    }
    else {
        // Set the session restart command the old way.
        auto clst = util::split(resdb().getResource("command"), " ");
        std::vector<char *> fargv(clst.size() + 1, 0);
        for (size_t i = 0 ; i < clst.size() ; i++) {
            if (i == 0)
                clst[i] = util::fs::findCommand(clst[i]);
            fargv[i] = const_cast<char *>(clst[i].c_str());
        }

        XSetWMProperties(display(), window(), &titlep, &iconnamep, fargv.data(),
          clst.size(), szHints, wmhints, classhints);
    }

    XFree(titlep.value);
    XFree(iconnamep.value);
    XFree(wmhints);
    XFree(classhints);

    // Set up the Atoms for delete messages
    _wm = XInternAtom(display(), "WM_PROTOCOLS", False);
    _wmdelete = XInternAtom(display(), "WM_DELETE_WINDOW", False);
    XChangeProperty(display(), window(), _wm, XA_ATOM, 32,
      PropModeReplace, (unsigned char *)(&_wmdelete), 1);
}


void XWin::setColors(void) {

    _visual = getVisual();

    if (XVisualIDFromVisual(visual())
      == XVisualIDFromVisual(DefaultVisual(display(), screen())))
        _colormap = DefaultColormap(display(), screen());
    else
        _colormap = XCreateColormap(display(), DefaultRootWindow(display()),
          visual(), AllocNone);

    if(_transparent)
        _bgColor = 0;
    else
        _bgColor = X11Graphics::allocColor(display(), colormap(),
          resdb().getResource("background"));

    _fgColor = X11Graphics::allocColor(display(), colormap(),
      resdb().getResource("foreground"));
}


XSizeHints *XWin::getGeometry(void) {
    // Fill out a XsizeHints structure to inform the window manager
    // of desired size and location of main window.
    XSizeHints *szHints = nullptr;
    if((szHints = XAllocSizeHints()) == nullptr) {
        logFatal << "Error allocating size hints!" << std::endl;
    }
    szHints->flags = PSize;
    szHints->height = height();
    szHints->min_height = szHints->height;
    szHints->width = width();
    szHints->min_width = szHints->width;
    szHints->x = _x;
    szHints->y = _y;

    // Construct a default geometry string
    std::ostringstream defgs;
    defgs << szHints->width << "x" << szHints->height << "+"
          << szHints->x << "+" << szHints->y;

    // Process the geometry specification
    std::string geomStr = resdb().getResourceOrUseDefault(
        "geometry", "<!UNSP!>");
    bool geomUnspecified = false;
    if (geomStr == "<!UNSP!>")
        geomUnspecified = true;
    const char *gptr = nullptr;
    if (!geomUnspecified)
        gptr = geomStr.c_str();
    int bitmask = XGeometry(display(), DefaultScreen(display()), gptr,
      defgs.str().c_str(),
      0,
      1, 1, 0, 0, &(szHints->x), &(szHints->y),
      &(szHints->width), &(szHints->height));

    // Check bitmask and set flags in XSizeHints structure
    if (bitmask & (WidthValue | HeightValue)) {
        szHints->flags |= PPosition;
        width(szHints->width);
        height(szHints->height);
    }

    if (bitmask & (XValue | YValue)) {
        szHints->flags |= USPosition;
        _x = szHints->x;
        _y = szHints->y;
    }

    return szHints;
}


void XWin::selectEvents(long mask) {
    XWindowAttributes    xAttr;
    XSetWindowAttributes xSwAttr;

    if (XGetWindowAttributes(display(), window(), &xAttr) != 0) {
        xSwAttr.event_mask = xAttr.your_event_mask | mask;
        XChangeWindowAttributes(display(), window(), CWEventMask, &xSwAttr);
    }
}


void XWin::ignoreEvents(long mask) {
    XWindowAttributes    xAttr;
    XSetWindowAttributes xSwAttr;

    if (XGetWindowAttributes(display(), window(), &xAttr) != 0) {
        xSwAttr.event_mask = xAttr.your_event_mask & mask;
        XChangeWindowAttributes(display(), window(), CWEventMask, &xSwAttr);
    }
}


void XWin::checkevent(void) {

    if (XEventsQueued(display(), QueuedAfterReading))
        logDebug << "++++++++++++ event sequence +++++++++++++" << std::endl;
    while (XEventsQueued(display(), QueuedAfterReading)) {
        XEvent event;
        XNextEvent(display(), &event);

        logDebug << "XWin::checkevent *** : " << event << std::endl;

        for (auto &evcb : _events) {
            evcb.callBack(event);
            if (done())
                return;
        }

        if (!XEventsQueued(display(), QueuedAfterReading))
            logDebug << "------------ event sequence -------------"
                     << std::endl;
    }
}


void XWin::addEvent(int eventType, const Callback &cb) {
    _events.emplace_back(eventType, cb);
}


void XWin::configureEvent(const XEvent &event) {
    _x = event.xconfigure.x;
    _y = event.xconfigure.y;
    width(event.xconfigure.width);
    height(event.xconfigure.height);
}


void XWin::deleteEvent(const XEvent &event) {
    if ((event.xclient.message_type == _wm) &&
      ((unsigned)event.xclient.data.l[0] == _wmdelete)) {
        logDebug << "calling done(true)..." << std::endl;
        done(true);
    }
}


long XWin::Event::mask(void) const {
    long mask = NoEventMask;
    switch (_event) {
    case ButtonPress:
        mask = ButtonPressMask;
        break;
    case ButtonRelease:
        mask = ButtonReleaseMask;
        break;
    case EnterNotify:
        mask = EnterWindowMask;
        break;
    case LeaveNotify:
        mask = LeaveWindowMask;
        break;
    case MotionNotify:
        mask = PointerMotionMask;
        break;
    case FocusIn:
    case FocusOut:
        mask = FocusChangeMask;
        break;
    case KeymapNotify:
        mask = KeymapStateMask;
        break;
    case KeyPress:
        mask = KeyPressMask;
        break;
    case KeyRelease:
        mask = KeyReleaseMask;
        break;
    case MapNotify:
    case SelectionClear:
    case SelectionNotify:
    case SelectionRequest:
    case ClientMessage:
    case MappingNotify:
        mask = NoEventMask;
        break;
    case Expose:
    case GraphicsExpose:
    case NoExpose:
        mask = ExposureMask;
        break;
    case ColormapNotify:
        mask = ColormapChangeMask;
        break;
    case PropertyNotify:
        mask = PropertyChangeMask;
        break;
    case UnmapNotify:
    case ReparentNotify:
    case GravityNotify:
    case DestroyNotify:
    case CirculateNotify:
    case ConfigureNotify:
        mask = StructureNotifyMask | SubstructureNotifyMask;
        break;
    case CreateNotify:
        mask = SubstructureNotifyMask;
        break;
    case VisibilityNotify:
        mask = VisibilityChangeMask;
        break;
        // The following are used by window managers
    case CirculateRequest:
    case ConfigureRequest:
    case MapRequest:
        mask = SubstructureRedirectMask;
        break;
    case ResizeRequest:
        mask = ResizeRedirectMask;
        break;
    default:
        logBug << "Unknown event type: " << _event << std::endl;
        mask = NoEventMask;
        break;
    }

    return mask;
}


static std::ostream &operator<<(std::ostream &os, const XEvent &e) {
    os << "XEvent: type=";
    switch (e.type) {
#ifdef XOSVDEBUG
    case KeyPress:
        os << "KeyPress";
        break;
    case KeyRelease:
        os << "KeyRelease";
        break;
    case ButtonPress:
        os << "ButtonPress";
        break;
    case ButtonRelease:
        os << "ButtonRelease";
        break;
    case MotionNotify:
        os << "MotionNotify";
        break;
    case EnterNotify:
        os << "EnterNotify";
        break;
    case LeaveNotify:
        os << "LeaveNotify";
        break;
    case FocusIn:
        os << "FocusIn";
        break;
    case FocusOut:
        os << "FocusOut";
        break;
    case KeymapNotify:
        os << "KeymapNotify";
        break;
    case Expose:
        os << "Expose";
        break;
    case GraphicsExpose:
        os << "GraphicsExpose";
        break;
    case NoExpose:
        os << "NoExpose";
        break;
    case CirculateRequest:
        os << "CirculateRequest";
        break;
    case ConfigureRequest:
        os << "ConfigureRequest";
        break;
    case MapRequest:
        os << "MapRequest";
        break;
    case ResizeRequest:
        os << "ResizeRequest";
        break;
    case CirculateNotify:
        os << "CirculateNotify";
        break;
    case ConfigureNotify:
        os << "ConfigureNotify";
        break;
    case CreateNotify:
        os << "CreateNotify";
        break;
    case DestroyNotify:
        os << "DestroyNotify";
        break;
    case GravityNotify:
        os << "GravityNotify";
        break;
    case MapNotify:
        os << "MapNotify";
        break;
    case MappingNotify:
        os << "MappingNotify";
        break;
    case ReparentNotify:
        os << "ReparentNotify";
        break;
    case UnmapNotify:
        os << "UnmapNotify";
        break;
    case VisibilityNotify:
        os << "VisibilityNotify";
        break;
    case ColormapNotify:
        os << "ColormapNotify";
        break;
    case ClientMessage:
        os << "ClientMessage";
        break;
    case PropertyNotify:
        os << "PropertyNotify";
        break;
    case SelectionClear:
        os << "SelectionClear";
        break;
    case SelectionNotify:
        os << "SelectionNotify";
        break;
    case SelectionRequest:
        os << "SelectionRequest";
        break;
#endif
    default:
        os << "Unknown(" << e.type << ")";
    };

    return os;
}


void XWin::resize(int width, int height) {
    XResizeWindow(display(), window(), width, height);
}


void XWin::map(void) {
    XMapWindow(display(), window());
}


void XWin::unmap(void) {
    XUnmapWindow(display(), window());
}


void XWin::mappingNotify(const XEvent &event) {
    XRefreshKeyboardMapping(const_cast<XMappingEvent *>(&event.xmapping));
}
