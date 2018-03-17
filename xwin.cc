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


XWin::XWin(void)
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

    // Define the windows attributes
    XSetWindowAttributes attr;
    unsigned long amask = 0;
    attr.colormap = colormap();           amask |= CWColormap;
    attr.border_pixel = background();     amask |= CWBorderPixel;
    attr.background_pixel = background(); amask |= CWBackPixel;
    attr.backing_store = WhenMapped;      amask |= CWBackingStore;
    attr.bit_gravity = ForgetGravity;     amask |= CWBitGravity;

    auto szHints = getGeometry();

    _window = XCreateWindow(display(), DefaultRootWindow(display()),
      szHints->x, szHints->y, szHints->width, szHints->height,
      0, depth(visual()), InputOutput, visual(), amask, &attr);

    equipWindow(std::move(szHints));
}


void XWin::equipWindow(x_unique_ptr<XSizeHints> &&szHints) {
    _bb = createBB();

    setHints(std::move(szHints));

    _graphics = createGraphics();

    setPixmapBG();

    // add the events
    for (const auto &event : _events)
        selectEvents(event.mask());

    map();
}


void XWin::setPixmapBG(void) {
    const std::string pixmapFName = resdb().getResourceOrUseDefault(
        "pixmapName", "");
    X11Pixmap x11p(display(), visual(), window(), colormap());
    if (!pixmapFName.empty() && x11p.load(pixmapFName))
        g().setBG(x11p);
}


std::unique_ptr<X11Graphics> XWin::createGraphics(void) const {

    auto g = std::make_unique<X11Graphics>(display(), visual(), _bb, true,
      colormap(), background());
    g->setFont(resdb().getResource("font"));
    g->setBG(background());
    g->setFG(foreground());
    g->setStippleMode(resdb().isResourceTrue("enableStipple"));

    return g;
}


Drawable XWin::createBB(void) const {
    Drawable bb = 0;
#ifdef HAVE_DBE
    if (_dbe) {
        bb = XdbeAllocateBackBufferName(display(), window(), XdbeCopied);
        logDebug << "BACK BUFFER ID: " << std::hex << std::showbase
                 << bb << std::endl;
    }
    else
        bb = window();
#else
    bb = window();
#endif

    return bb;
}


int XWin::depth(Visual *v) const {
    XVisualInfo tmplt;
    tmplt.visualid = XVisualIDFromVisual(v);
    int ninfo = 0;
    x_unique_ptr<XVisualInfo> vinfo(XGetVisualInfo(display(), VisualIDMask,
        &tmplt, &ninfo), XFree);

    if (!vinfo || ninfo != 1)
        logFatal << "Failed to locate XVisualInfo for: "
                 << std::hex << std::showbase << tmplt.visualid << std::endl;

    return vinfo->depth;
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
        x2_unique_ptr<XdbeScreenVisualInfo> dbeVisuals(XdbeGetVisualInfo(
              display(), &rootWindow, &numsp), XdbeFreeVisualInfo);

        if (dbeVisuals)
            for (int i = 0 ; i < dbeVisuals->count ; i++) {
                if (visID == dbeVisuals->visinfo[i].visual) {
                    rval = true;
                    break;
                }
            }
    }
#else
    (void) v; // avoid the warning
#endif

    return rval;
}


void XWin::setEvents(void) {
    // Set up the default Events.
    // (the this->METHOD() thing is a gcc bug workaround.
    addEvent(ConfigureNotify, [this](auto e){ this->configureEvent(e); });
    addEvent(ClientMessage, [this](auto e){ this->deleteEvent(e); });
    addEvent(MappingNotify, [this](auto e){ this->mappingNotify(e); });
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


void XWin::setHints(x_unique_ptr<XSizeHints> &&szHints) {
    // Set up the window manager hints
    x_unique_ptr<XWMHints> wmhints(XAllocWMHints(), XFree);
    if (!wmhints)
        logFatal << "Error allocating Window Manager hints!" << std::endl;

    wmhints->flags = InputHint | StateHint;
    wmhints->input = True;
    wmhints->initial_state = resdb().isResourceTrue("iconic") ? IconicState
        : NormalState;

    // Set up XTextProperty for window name and icon name.
    // XStringListToTextProperty allocates memory.
    // So, titlep.value and iconnamep.value need XFree.
    char *np = const_cast<char *>(_name.c_str());
    XTextProperty titlep;
    if (XStringListToTextProperty(&np, 1, &titlep) == 0)
        logFatal << "Error creating XTextProperty!" << std::endl;

    XTextProperty iconnamep;
    if (XStringListToTextProperty(&np, 1, &iconnamep) == 0)
        logFatal << "Error creating XTextProperty!" << std::endl;

    // Set up class hint for window manager.
    x_unique_ptr<XClassHint> classhints(XAllocClassHint(), XFree);
    if(!classhints)
        logFatal << "Error allocating class hint!" << std::endl;
    const std::string cname = resdb().className();
    const std::string iname = resdb().instanceName();
    classhints->res_name = const_cast<char *>(iname.c_str());
    classhints->res_class = const_cast<char *>(cname.c_str());

    // Set the command line based on the use of session management.
    if (!resdb().getResource("sessionID").empty()) {
        // X11R6 Session Manager gave us an ID.  Command handled elsewhere.
        XSetWMProperties(display(), window(), &titlep, &iconnamep, nullptr,
          0, szHints.get(), wmhints.get(), classhints.get());
    }
    else {
        // Set the session restart command the old way.
        auto clst = util::split(resdb().getResource("command"), " ");
        std::vector<char *> fargv(clst.size() + 1, nullptr);
        for (size_t i = 0 ; i < clst.size() ; i++) {
            if (i == 0)
                clst[i] = util::fs::findCommand(clst[i]);
            fargv[i] = const_cast<char *>(clst[i].c_str());
        }

        XSetWMProperties(display(), window(), &titlep, &iconnamep, fargv.data(),
          clst.size(), szHints.get(), wmhints.get(), classhints.get());
    }

    XFree(titlep.value);
    XFree(iconnamep.value);

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


XWin::x_unique_ptr<XSizeHints> XWin::getGeometry(void) {
    // Fill out a XsizeHints structure to inform the window manager
    // of desired size and location of main window.
    x_unique_ptr<XSizeHints> szHints(XAllocSizeHints(), XFree);
    if(!szHints)
        logFatal << "Error allocating size hints!" << std::endl;

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
    const char *gptr = geomStr != "<!UNSP!>" ? geomStr.c_str() : nullptr;
    const int bitmask = XGeometry(display(), DefaultScreen(display()), gptr,
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
    XWindowAttributes xAttr;
    if (XGetWindowAttributes(display(), window(), &xAttr) != 0) {
        XSetWindowAttributes xSwAttr;
        xSwAttr.event_mask = xAttr.your_event_mask | mask;
        XChangeWindowAttributes(display(), window(), CWEventMask, &xSwAttr);
    }
}


void XWin::ignoreEvents(long mask) {
    XWindowAttributes xAttr;
    if (XGetWindowAttributes(display(), window(), &xAttr) != 0) {
        XSetWindowAttributes xSwAttr;
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
      (static_cast<unsigned>(event.xclient.data.l[0]) == _wmdelete)) {
        logDebug << "calling done(true)..." << std::endl;
        done(true);
    }
}


long XWin::Event::mask(void) const {
    // table mapping event type to mask that enables it.
    const std::map<int, long> emap = {
        {ButtonPress, ButtonPressMask},
        {ButtonRelease, ButtonReleaseMask},
        {EnterNotify, EnterWindowMask},
        {LeaveNotify, LeaveWindowMask},
        {MotionNotify, PointerMotionMask},
        {FocusIn, FocusChangeMask},
        {FocusOut, FocusChangeMask},
        {KeymapNotify, KeymapStateMask},
        {KeyPress, KeyPressMask},
        {KeyRelease, KeyReleaseMask},
        {MapNotify, NoEventMask},
        {SelectionClear, NoEventMask},
        {SelectionNotify, NoEventMask},
        {SelectionRequest, NoEventMask},
        {ClientMessage, NoEventMask},
        {MappingNotify, NoEventMask},
        {Expose, ExposureMask},
        {GraphicsExpose, ExposureMask},
        {NoExpose, ExposureMask},
        {ColormapNotify, ColormapChangeMask},
        {PropertyNotify, PropertyChangeMask},
        {UnmapNotify, StructureNotifyMask | SubstructureNotifyMask},
        {ReparentNotify, StructureNotifyMask | SubstructureNotifyMask},
        {GravityNotify, StructureNotifyMask | SubstructureNotifyMask},
        {DestroyNotify, StructureNotifyMask | SubstructureNotifyMask},
        {CirculateNotify, StructureNotifyMask | SubstructureNotifyMask},
        {ConfigureNotify, StructureNotifyMask | SubstructureNotifyMask},
        {CreateNotify, SubstructureNotifyMask},
        {VisibilityNotify, VisibilityChangeMask},
        // The following are used by window managers
        {CirculateRequest, SubstructureRedirectMask},
        {ConfigureRequest, SubstructureRedirectMask},
        {MapRequest, SubstructureRedirectMask},
        {ResizeRequest, ResizeRedirectMask}
    };

    auto it = emap.find(_event);
    if (it != emap.end())
        return it->second;

    logBug << "Unknown event type: " << _event << std::endl;
    return NoEventMask;
}


static std::ostream &operator<<(std::ostream &os, const XEvent &e) {

#ifdef XOSVDEBUG
    // map of eventType=>string.
#define MK_TUP(arg) {arg, #arg}
    const std::map<int, std::string> emap = {
        MK_TUP(KeyPress), MK_TUP(KeyRelease), MK_TUP(ButtonPress),
        MK_TUP(ButtonRelease), MK_TUP(MotionNotify), MK_TUP(EnterNotify),
        MK_TUP(LeaveNotify), MK_TUP(FocusIn), MK_TUP(FocusOut),
        MK_TUP(KeymapNotify), MK_TUP(Expose), MK_TUP(GraphicsExpose),
        MK_TUP(NoExpose), MK_TUP(CirculateRequest), MK_TUP(ConfigureRequest),
        MK_TUP(MapRequest), MK_TUP(ResizeRequest), MK_TUP(CirculateNotify),
        MK_TUP(ConfigureNotify), MK_TUP(CreateNotify), MK_TUP(DestroyNotify),
        MK_TUP(GravityNotify), MK_TUP(MapNotify), MK_TUP(MappingNotify),
        MK_TUP(ReparentNotify), MK_TUP(UnmapNotify), MK_TUP(VisibilityNotify),
        MK_TUP(ColormapNotify), MK_TUP(ClientMessage), MK_TUP(PropertyNotify),
        MK_TUP(SelectionClear), MK_TUP(SelectionNotify),
        MK_TUP(SelectionRequest)
    };
#undef MK_TUP

    os << "XEvent: type=";
    auto it = emap.find(e.type);
    if (it != emap.end())
        os << it->second;
    else
        os << "Unknown(" << e.type << ")";
#else
    os << "XEvent: type=" << "Unknown(" << e.type << ")";
#endif // XOSVDEBUG

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
