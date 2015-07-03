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

#include <X11/Xatom.h>
#ifdef HAVE_XPM
#include <X11/xpm.h>
#endif


static std::ostream &operator<<(std::ostream &os, const XEvent &e);


XWin::XWin() : _graphics(0), events_(0), done_(false),
               wm_(None), wmdelete_(None), x_(0), y_(0), width_(1), height_(1),
               display_(0), window_(0), fgcolor_(0), bgcolor_(0),
               colormap_(0) {
}

void XWin::XWinInit(int argc, char** argv) {
    (void) argc;
    (void) argv;  //  Avoid gcc warnings about unused variables.
    //  Eventually, we may want to have XWin handle some arguments other
    //  than resources, so argc and argv are left as parameters.  BCG

    width_ = height_ = x_ = y_ = 0;
    done_ = false;

    // Set up the default Events
    events_ = NULL;
    addEvent( new Event( this, ConfigureNotify, &XWin::configureEvent ) );
    addEvent( new Event( this, ClientMessage, &XWin::deleteEvent ) );
    addEvent( new Event( this, MappingNotify, &XWin::mappingNotify ) );

    //openDisplay();  //  Done explicitly in xosview.cc.
}

XWin::~XWin( void ){
    // remove the Graphics interface
    delete _graphics;
    _graphics = 0;

    // delete the events
    Event *event = events_;
    while ( event != NULL ){
        Event *save = event->next_;
        delete event;
        event = save;
    }

    XDestroyWindow( display_, window_ );
    // close the connection to the display
    XCloseDisplay( display_ );
}


void XWin::init(int argc, char **argv, const std::string &pixmapFName,
  const std::string &geomStr, bool geomUnspecified){
    XSetWindowAttributes xswa;

    std::string fontName = getResource("font");
    setColors();
    XSizeHints *szHints = getGeometry(geomStr, geomUnspecified);

    window_ = XCreateSimpleWindow(display_, DefaultRootWindow(display_),
      szHints->x, szHints->y,
      szHints->width, szHints->height,
      1,
      fgcolor_, bgcolor_);

    setHints( argc, argv, szHints );
    XFree(szHints);
    szHints = 0;

    // Set main window's attributes (colormap, bit_gravity)
    xswa.colormap = colormap_;
    xswa.bit_gravity = NorthWestGravity;
    XChangeWindowAttributes(display_, window_,
      (CWColormap | CWBitGravity), &xswa);

#ifdef HAVE_XPM
    Pixmap	       background_pixmap;
    bool doPixmap = getPixmap(&background_pixmap, pixmapFName);
    // If there is a pixmap file, set it as the background
    if(doPixmap) {
	XSetWindowBackgroundPixmap(display_,window_,background_pixmap);
    }
#else
    util::toupper(pixmapFName); // just to avoid warning about unused.
#endif

    // Do transparency if requested
    if(isResourceTrue("transparent")) {
        XSetWindowBackgroundPixmap(display_,window_,ParentRelative);
    }

    // add the events
    Event *tmp = events_;
    while ( tmp != NULL ){
        selectEvents( tmp->mask_ );
        tmp = tmp->next_;
    }

    // Create new Graphics interface.
    _graphics = new X11Graphics(display_, window_, true, colormap_,
      bgcolor_);
    g().setFont(fontName);
    g().setBG(bgcolor_);
    g().setFG(fgcolor_);
    g().setStippleMode(isResourceTrue("enableStipple"));

    // Map the main window
    map();
    g().flush();
}

void XWin::setHints(int argc, char *argv[], XSizeHints *szHints){
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

    XSetWMProperties(display_, window_, &titlep, &iconnamep, argv, argc,
      szHints, wmhints, classhints);
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

void XWin::openDisplay( void ){
    // Open connection to display selected by user
    if ((display_ = XOpenDisplay (display_name_.c_str())) == NULL) {
        logFatal << "Can't open display named " << display_name_ << std::endl;
    }

    colormap_ = DefaultColormap( display_, screen() );
}

void XWin::setColors( void ){
    XColor               color;

    // Main window's background color
    if (XParseColor(display_, colormap_,
        getResource("background").c_str(),
        &color) == 0 ||
      XAllocColor(display_, colormap_, &color) == 0)
        bgcolor_ = WhitePixel(display_, DefaultScreen(display_));
    else
        bgcolor_ = color.pixel;

    // Main window's foreground color */
    if (XParseColor(display_, colormap_,
        getResource("foreground").c_str(),
        &color) == 0 ||
      XAllocColor(display_, colormap_, &color) == 0)
        fgcolor_ = BlackPixel(display_, DefaultScreen(display_));
    else
        fgcolor_ = color.pixel;
}

#ifdef HAVE_XPM
bool XWin::getPixmap(Pixmap *pixmap, const std::string &pixmapFName) {
    if (pixmapFName == "") {
        pixmap = NULL;
        return false;
    }

    XWindowAttributes    root_att;
    XpmAttributes        pixmap_att;

    XGetWindowAttributes(display_, DefaultRootWindow(display_),&root_att);
    pixmap_att.closeness=30000;
    pixmap_att.colormap=root_att.colormap;
    pixmap_att.valuemask=XpmSize|XpmReturnPixels|XpmColormap|XpmCloseness;
    if(XpmReadFileToPixmap(display_,DefaultRootWindow(display_),
        pixmapFName.c_str(), pixmap, NULL, &pixmap_att)) {
        logProblem << "Pixmap " << pixmapFName << " not found"
                   << std::endl
                   << "Defaulting to blank" << std::endl;
        pixmap=NULL;
        return false; // OOps
    }
    return true;  // Good, found the pixmap
}
#endif

XSizeHints *XWin::getGeometry(const std::string &geomStr, bool geomUnspecified){
    int                  bitmask;

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
    const char *gptr = NULL;
    if (!geomUnspecified)
        gptr = geomStr.c_str();
    bitmask =  XGeometry(display_, DefaultScreen(display_), gptr,
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
    XEvent event;

    while ( !done() && XEventsQueued( display_, QueuedAfterReading ) ){
        XNextEvent( display_, &event );
        logDebug << "EVENT: " << event << ", done=" << done() << std::endl;

        // call all of the Event's call back functions to process this event
        Event *tmp = events_;
        while ( !done() && (tmp != NULL) ){
            tmp->callBack( event );
            tmp = tmp->next_;
        }
    }
}

#if 0
void XWin::usage( void ){
    //  FIXME  We need to update this.  BCG
    std::cout << name_ << " [-display name] [-geometry geom]" << std::endl;
//    exit (1);
}
#endif


void XWin::addEvent( Event *event ){
    Event *tmp = events_;

    if ( events_ == NULL )
        events_ = event;
    else {
        while ( tmp->next_ != NULL )
            tmp = tmp->next_;
        tmp->next_ = event;
    }
}

void XWin::configureEvent( XEvent &event ){
    x( event.xconfigure.x );
    y( event.xconfigure.y );
    width( event.xconfigure.width );
    height( event.xconfigure.height );
}

void XWin::deleteEvent( XEvent &event ){
    if ( (event.xclient.message_type == wm_ ) &&
      ((unsigned)event.xclient.data.l[0] == wmdelete_) ) {
        logDebug << "calling done(true)..." << std::endl;
        done(true);
    }
}

XWin::Event::Event( XWin *parent, int event, EventCallBack callBack ){
    next_ = NULL;
    parent_ = parent;
    event_ = event;
    callBack_ = callBack;

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
    return new X11Pixmap(display_, window_,
      colormap_, bgcolor_, width, height, g().depth());
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

std::string XWin::className(void) {
    logProblem << "className() not overridden using killroy...\n";
    return "killroy";
}

std::string XWin::instanceName(void) {
    logProblem << "instanceName() not overridden using killroy...\n";
    return "killroy";
}
