//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "xwin.h"
#include "Xrm.h"
#include "log.h"
#include "x11pixmap.h"
#include "strutil.h"

#include <sstream>
#include <algorithm>

#include <X11/Xatom.h>
#ifdef HAVE_DBE
#include <X11/extensions/Xdbe.h>
#endif


static std::ostream &operator<<(std::ostream &os, const XEvent &e);


XWin::XWin()
    : _graphics(0), done_(false),
      wm_(None), wmdelete_(None), x_(0), y_(0), width_(1), height_(1),
      visual_(0), display_(0), window_(0), fgcolor_(0), bgcolor_(0),
      colormap_(0), _dbe(false), _bb(0), _bgw(0) {
}


XWin::~XWin( void ){
    // remove the Graphics interface
    delete _graphics;
    _graphics = 0;

#ifdef HAVE_DBE
    if (_dbe && _bb)
        XdbeDeallocateBackBufferName(display_, _bb);
#endif
    if (_bgw)
        XDestroyWindow( display_, _bgw );

    XDestroyWindow( display_, window_ );
    // close the connection to the display
    XCloseDisplay( display_ );
}


void XWin::openDisplay( void ){
    // Open connection to display selected by user
    const char *dname = display_name_ == "" ? NULL : display_name_.c_str();
    if ((display_ = XOpenDisplay(dname)) == NULL) {
        logFatal << "Can't open display named "
                 << "'" << dname << "'" << std::endl;
    }
}


void XWin::createWindow(void) {

    visual_ = getVisual();
    colormap_ = DefaultColormap( display_, screen() );
    setColors();

    XSizeHints *szHints = getGeometry();

    XVisualInfo tmplt;
    tmplt.visualid = XVisualIDFromVisual(visual_);
    int ninfo = 0;
    XVisualInfo *vinfo = XGetVisualInfo(display_, VisualIDMask, &tmplt,
      &ninfo);
    if (ninfo != 1)
        logFatal << "Failed to locate XVisualInfo." << std::endl;

    // Define the windows attributes
    XSetWindowAttributes attr;
    unsigned long amask = 0;
    attr.colormap = colormap_;            amask |= CWColormap;
    attr.border_pixel = fgcolor_;         amask |= CWBorderPixel;
    attr.background_pixel = bgcolor_;     amask |= CWBackPixel;
    attr.backing_store = WhenMapped;      amask |= CWBackingStore;
    attr.bit_gravity = ForgetGravity;     amask |= CWBitGravity;

    window_ = XCreateWindow(display_, DefaultRootWindow(display_),
      szHints->x, szHints->y, szHints->width, szHints->height,
      0, vinfo->depth, InputOutput, visual_, amask, &attr);
    if (_dbe) {
        // Make a "stealth" companion window that is never
        // mapped.  It exists to hold the background pixmap (if any)
        // So, we can use an alternative to XClearArea().
        _bgw = XCreateWindow(display_, DefaultRootWindow(display_),
          szHints->x, szHints->y, szHints->width, szHints->height,
          0, vinfo->depth, InputOutput, visual_, amask, &attr);
    }
    XFree(vinfo);

#ifdef HAVE_DBE
    if (_dbe) {
        _bb = XdbeAllocateBackBufferName(display_, window_, XdbeCopied);
        logDebug << "BACK BUFFER ID: " << std::hex << std::showbase
                 << _bb << std::endl;
    }
    else
        _bb = window_;
#else
    _bb = window_;
#endif

    setHints(szHints);
    XFree(szHints);
    szHints = 0;

    // Pixmap backgrounds
    std::string pixmapFName = getResourceOrUseDefault("pixmapName", "");
    X11Pixmap x11p(display_, visual_, window_, colormap_);
    if (pixmapFName.size() && x11p.load(pixmapFName)) {
	XSetWindowBackgroundPixmap(display_, window_, x11p.pmap());
        if (_bgw)
            XSetWindowBackgroundPixmap(display_, _bgw, x11p.pmap());
    }

    if(isResourceTrue("transparent")) {
        XSetWindowBackgroundPixmap(display_, window_, ParentRelative);
        if (_bgw)
            XSetWindowBackgroundPixmap(display_, _bgw, ParentRelative);
    }

    // add the events
    for (size_t i = 0 ; i < events_.size() ; i++)
        selectEvents(events_[i].mask_);

    // Create new Graphics interface.
    _graphics = new X11Graphics(display_, visual_, _bb, true, colormap_,
      bgcolor_);
    g().setFont(getResource("font"));
    g().setBG(bgcolor_);
    g().setFG(fgcolor_);
    g().setStippleMode(isResourceTrue("enableStipple"));

    // Map the main window
    map();
    //g().flush();
}


Visual *XWin::getVisual(void) {

    Visual *defaultVisual = DefaultVisual(display_, screen());
    _dbe = isDBE(defaultVisual);
    if (_dbe)
        logDebug << "Enabling DBE..." << std::endl;
    return defaultVisual;
}


bool XWin::isDBE(Visual *v) const {
    bool rval = false;

#ifdef HAVE_DBE
    VisualID visID = XVisualIDFromVisual(v);

    int major, minor;
    if (XdbeQueryExtension(display_, &major, &minor)) {
        logDebug << "HAVE DBE: " << major << "." << minor << std::endl;

        Window rootWindow = XDefaultRootWindow(display_);
        int numsp = 1;
        XdbeScreenVisualInfo *dbeVisuals = XdbeGetVisualInfo(display_,
          &rootWindow, &numsp);

        if (dbeVisuals != NULL) {
            for (int i = 0 ; i < dbeVisuals->count ; i++) {
                if (visID == dbeVisuals->visinfo[i].visual) {
                    rval = true;
                    break;
                }
            }
            XdbeFreeVisualInfo(dbeVisuals);
        }
    }
#endif

    return rval;
}


void XWin::setEvents(void) {
    // Set up the default Events
    addEvent( ConfigureNotify, this, &XWin::configureEvent );
    addEvent( ClientMessage, this, &XWin::deleteEvent );
    addEvent( MappingNotify, this, &XWin::mappingNotify  );
}


void XWin::swapBB(void) const {
#ifdef HAVE_DBE
    if (_dbe) {
        XdbeSwapInfo swinfo;
        swinfo.swap_window = window_;
        swinfo.swap_action = XdbeCopied;
        XdbeSwapBuffers(display_, &swinfo, 1);
    }
#endif
}


void XWin::setHints(XSizeHints *szHints){
    // Set up class hint
    XClassHint    *classhints;   //  Class hint for window manager
    if((classhints = XAllocClassHint()) == NULL){
        logFatal << "Error allocating class hint!" << std::endl;
    }
    //  We have to cast away the const's.
    std::string cname = className();
    std::string iname = instanceName();
    classhints->res_name = const_cast<char *>(iname.c_str());
    classhints->res_class = const_cast<char *>(cname.c_str());

    // Set up the window manager hints
    XWMHints      *wmhints;      //  Hints for the window manager
    if((wmhints = XAllocWMHints()) == NULL){
        logFatal << "Error allocating Window Manager hints!" << std::endl;
    }

    wmhints->flags = (InputHint|StateHint);
    wmhints->input = True;
    wmhints->initial_state = NormalState;
    if (isResourceTrue("iconic"))
        wmhints->initial_state = IconicState;

    // Set up XTextProperty for window name and icon name
    char *np = const_cast<char *>(name_.c_str());
    XTextProperty titlep;
    if(XStringListToTextProperty(&np, 1, &titlep) == 0){
        logFatal << "Error creating XTextProperty!" << std::endl;
    }
    XTextProperty iconnamep;
    if(XStringListToTextProperty(&np, 1, &iconnamep) == 0){
        logFatal << "Error creating XTextProperty!" << std::endl;
    }

    // First make a "fake" argument list
    std::vector<std::string> clst = util::split(getResource("command"), " ");
    char **fargv = new char*[clst.size()+1];
    for (size_t i = 0 ; i < clst.size() ; i++)
        fargv[i] = const_cast<char *>(clst[i].c_str()); // we'll be careful...
    fargv[clst.size()] = 0; // just in cat argc is ignored

    XSetWMProperties(display_, window_, &titlep, &iconnamep, fargv,
      clst.size(), szHints, wmhints, classhints);

    delete[] fargv; // the char* elements here just point into clst
    fargv = 0;

    XFree( titlep.value );
    XFree( iconnamep.value );

    // Set up the Atoms for delete messages
    wm_ = XInternAtom( display(), "WM_PROTOCOLS", False );
    wmdelete_ = XInternAtom( display(), "WM_DELETE_WINDOW", False );
    XChangeProperty( display(), window(), wm_, XA_ATOM, 32,
      PropModeReplace, (unsigned char *)(&wmdelete_), 1 );

    XFree(wmhints);
    XFree(classhints);
}


void XWin::setColors( void ){
    bgcolor_ = X11Graphics::allocColor(display_, colormap_,
      getResource("background"));

    fgcolor_ = X11Graphics::allocColor(display_, colormap_,
      getResource("foreground"));
}


XSizeHints *XWin::getGeometry(void) {

    // Fill out a XsizeHints structure to inform the window manager
    // of desired size and location of main window.
    XSizeHints *szHints = 0;
    if((szHints = XAllocSizeHints()) == NULL){
        logFatal << "Error allocating size hints!" << std::endl;
    }
    szHints->flags = PSize;
    szHints->height = height_;
    szHints->min_height = szHints->height;
    szHints->width = width_;
    szHints->min_width = szHints->width;
    szHints->x = x_;
    szHints->y = y_;

    // Construct a default geometry string
    std::ostringstream defgs;
    defgs << szHints->width << "x" << szHints->height << "+"
          << szHints->x << "+" << szHints->y;

    // Process the geometry specification
    std::string geomStr = getResourceOrUseDefault("geometry", "<!UNSP!>");
    bool geomUnspecified = false;
    if (geomStr == "<!UNSP!>")
        geomUnspecified = true;
    const char *gptr = NULL;
    if (!geomUnspecified)
        gptr = geomStr.c_str();
    int bitmask =  XGeometry(display_, DefaultScreen(display_), gptr,
      defgs.str().c_str(),
      0,
      1, 1, 0, 0, &(szHints->x), &(szHints->y),
      &(szHints->width), &(szHints->height));

    // Check bitmask and set flags in XSizeHints structure
    if (bitmask & (WidthValue | HeightValue)){
        szHints->flags |= PPosition;
        width_ = szHints->width;
        height_ = szHints->height;
    }

    if (bitmask & (XValue | YValue)){
        szHints->flags |= USPosition;
        x_ = szHints->x;
        y_ = szHints->y;
    }

    return szHints;
}


void XWin::selectEvents( long mask ){
    XWindowAttributes    xAttr;
    XSetWindowAttributes xSwAttr;

    if ( XGetWindowAttributes( display_, window_, &xAttr ) != 0 ){
        xSwAttr.event_mask = xAttr.your_event_mask | mask;
        XChangeWindowAttributes( display_, window_, CWEventMask, &xSwAttr );
    }
}


void XWin::ignoreEvents( long mask ){
    XWindowAttributes    xAttr;
    XSetWindowAttributes xSwAttr;

    if ( XGetWindowAttributes( display_, window_, &xAttr ) != 0 ){
        xSwAttr.event_mask = xAttr.your_event_mask & mask;
        XChangeWindowAttributes( display_, window_, CWEventMask, &xSwAttr );
    }
}


void XWin::checkevent( void ){

    while (XEventsQueued(display_, QueuedAfterReading)) {
        XEvent event;
        XNextEvent(display_, &event);

        logDebug << "EVENT: " << event << std::endl;

        for (size_t j = 0 ; j < events_.size() ; j++) {
            events_[j].callBack(event);
            if (done())
                return;
        }
    }
}


void XWin::addEvent(int eventType, XWin *xwin, EventCallBack callBack){
    events_.push_back(Event(xwin, eventType, callBack));
}


void XWin::configureEvent( XEvent &event ){
    unsigned int origw = width();
    unsigned int origh = height();
    x( event.xconfigure.x );
    y( event.xconfigure.y );
    width( event.xconfigure.width );
    height( event.xconfigure.height );
    if ((width() != origw) || (height() != origh)) {
        logDebug << "XWin::configureEvent(): Dimension change.  CLEAR"
                 << std::endl;
        logDebug << std::hex << std::showbase
                 << "Window ID: " << window_ << ", "
                 << "BB ID: " << _bb << std::endl;
        XClearWindow(display_, window_);

        if (_dbe) {
            // The DBE back buffer is not being cleared by the above
            // XClearWindow() like the docs say.  This seems to do it.
            logDebug << "DBE Swap (XdbeBackground)." << std::endl;
            XdbeSwapInfo swinfo;
            swinfo.swap_window = window_;
            swinfo.swap_action = XdbeBackground;
            XdbeSwapBuffers(display_, &swinfo, 1);
        }
    }
}


void XWin::deleteEvent( XEvent &event ){
    if ( (event.xclient.message_type == wm_ ) &&
      ((unsigned)event.xclient.data.l[0] == wmdelete_) ) {
        logDebug << "calling done(true)..." << std::endl;
        done(true);
    }
}


XWin::Event::Event( XWin *parent, int event, EventCallBack callBack )
    : parent_(parent), callBack_(callBack), event_(event),
      mask_(NoEventMask) {

    switch ( event_ ){
    case ButtonPress:
        mask_ = ButtonPressMask;
        break;
    case ButtonRelease:
        mask_ = ButtonReleaseMask;
        break;
    case EnterNotify:
        mask_ = EnterWindowMask;
        break;
    case LeaveNotify:
        mask_ = LeaveWindowMask;
        break;
    case MotionNotify:
        mask_ = PointerMotionMask;
        break;
    case FocusIn:
    case FocusOut:
        mask_ = FocusChangeMask;
        break;
    case KeymapNotify:
        mask_ = KeymapStateMask;
        break;
    case KeyPress:
        mask_ = KeyPressMask;
        break;
    case KeyRelease:
        mask_ = KeyReleaseMask;
        break;
    case MapNotify:
    case SelectionClear:
    case SelectionNotify:
    case SelectionRequest:
    case ClientMessage:
    case MappingNotify:
        mask_ = NoEventMask;
        break;
    case Expose:
    case GraphicsExpose:
    case NoExpose:
        mask_ = ExposureMask;
        break;
    case ColormapNotify:
        mask_ = ColormapChangeMask;
        break;
    case PropertyNotify:
        mask_ = PropertyChangeMask;
        break;
    case UnmapNotify:
    case ReparentNotify:
    case GravityNotify:
    case DestroyNotify:
    case CirculateNotify:
    case ConfigureNotify:
        mask_ = StructureNotifyMask | SubstructureNotifyMask;
        break;
    case CreateNotify:
        mask_ = SubstructureNotifyMask;
        break;
    case VisibilityNotify:
        mask_ = VisibilityChangeMask;
        break;
        // The following are used by window managers
    case CirculateRequest:
    case ConfigureRequest:
    case MapRequest:
        mask_ = SubstructureRedirectMask;
        break;
    case ResizeRequest:
        mask_ = ResizeRedirectMask;
        break;
    default:
        logBug << "XWin::Event::Event() : unknown event type : "
               << event_ << std::endl;
        mask_ = NoEventMask;
        break;
    }
}


X11Pixmap *XWin::newX11Pixmap(unsigned int width, unsigned int height) {
    return new X11Pixmap(display_, visual_, _bb,
      colormap_, bgcolor_, width, height, g().depth());
}


std::vector<XVisualInfo> XWin::getVisuals(void) {
    std::vector<XVisualInfo> rval;

    XVisualInfo *visList, visFilter;
    int numVis;
    visFilter.screen = DefaultScreen(display_);
    visList = XGetVisualInfo(display_, VisualScreenMask, &visFilter, &numVis);
    for (int i = 0 ; i < numVis ; i++)
        rval.push_back(visList[i]);

    if (numVis)
        XFree(visList);

    return rval;
}


//-------------------------------------------------------------------
// These are just stubs to make XWin stand alone if a derrived class
// does not implement a resource data base
//-------------------------------------------------------------------
std::string XWin::getResource(const std::string &name) {
    if (name == "transparent")
        return "True";
    else if (name == "enableStipple")
        return "True";
    else if (name == "font")
        return "fixed";
    else if (name == "background")
        return "black";
    else if (name == "foreground")
        return "white";
    logFatal << "Resource " << name << "not found." << std::endl;
    return "";
}


std::string XWin::getResourceOrUseDefault(const std::string &,
  const std::string &) {
    logFatal << "NOT IMPLEMENTED" << std::endl;
    return "";
}


bool XWin::isResourceTrue(const std::string &name) {
    return util::tolower(getResource(name)) == "true";
}


void XWin::dumpResources(std::ostream &) {
    logFatal << "not implemented." << std::endl;
}


std::string XWin::className(void) {
    logProblem << "className() not overridden using killroy...\n";
    return "killroy";
}


std::string XWin::instanceName(void) {
    logProblem << "instanceName() not overridden using killroy...\n";
    return "killroy";
}
//-------------------------------------------------------------------

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
