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


static std::ostream &operator<<(std::ostream &os, const XEvent &e);


XWin::XWin()
    : _graphics(0), done_(false),
      wm_(None), wmdelete_(None), x_(0), y_(0), width_(1), height_(1),
      display_(0), window_(0), fgcolor_(0), bgcolor_(0),
      colormap_(0) {
}


XWin::~XWin( void ){
    // remove the Graphics interface
    delete _graphics;
    _graphics = 0;

    XDestroyWindow( display_, window_ );
    // close the connection to the display
    XCloseDisplay( display_ );
}


void XWin::setEvents(void) {
    // Set up the default Events
    addEvent( ConfigureNotify, this, &XWin::configureEvent );
    addEvent( ClientMessage, this, &XWin::deleteEvent );
    addEvent( MappingNotify, this, &XWin::mappingNotify  );
}


void XWin::createWindow(void) {
    XSetWindowAttributes xswa;

    std::string fontName = getResource("font");
    setColors();
    XSizeHints *szHints = getGeometry();

    window_ = XCreateSimpleWindow(display_, DefaultRootWindow(display_),
      szHints->x, szHints->y,
      szHints->width, szHints->height,
      1,
      fgcolor_, bgcolor_);

    setHints(szHints);
    XFree(szHints);
    szHints = 0;

    // Set main window's attributes (colormap, bit_gravity)
    xswa.colormap = colormap_;
    xswa.bit_gravity = NorthWestGravity;
    XChangeWindowAttributes(display_, window_,
      (CWColormap | CWBitGravity), &xswa);

    // If there is a pixmap file, set it as the background
    std::string pixmapFName = getResourceOrUseDefault("pixmapName", "");
    X11Pixmap x11p(display_, window_, colormap_);
    if (pixmapFName.size() && x11p.load(pixmapFName))
	XSetWindowBackgroundPixmap(display_, window_, x11p.pmap());

    // Do transparency if requested
    if(isResourceTrue("transparent")) {
        XSetWindowBackgroundPixmap(display_,window_,ParentRelative);
    }

    // add the events
    for (size_t i = 0 ; i < events_.size() ; i++)
        selectEvents(events_[i].mask_);

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


std::vector<XEvent> XWin::filterQueue(std::vector<XEvent> &queue) const {
    // Return a queue where queue[0] is the oldest event
    // Only include the most recent event of certain types.
    std::vector<XEvent> rval;

    // Include only the most recent of these types
    std::vector<std::pair<int,bool> > pone;
    pone.push_back(std::make_pair(ConfigureNotify, false));
    pone.push_back(std::make_pair(Expose, false));
    pone.push_back(std::make_pair(VisibilityNotify, false));

    for (size_t i = 0 ; i < queue.size() ; i++) {
        // Check each filter
        bool filteredType = false;
        for (size_t j = 0 ; j < pone.size() ; j++) {
            if (pone[j].first == queue[i].type) {
                if (!pone[j].second) {
                    // push just the first one found
                    pone[j].second = true;
                    rval.push_back(queue[i]);
                }
                filteredType = true; // mark it as being a filtered type
                break;
            }
        }
        // always add all non filtered types
        if (!filteredType)
            rval.push_back(queue[i]);
    }

    return rval;
}

void XWin::checkevent( void ){

    int qsize = XEventsQueued(display_, QueuedAfterReading);

    if (qsize) {
        // pull the queue and store it so queue[0] is newest
        std::vector<XEvent> queue(qsize);
        for (std::vector<XEvent>::reverse_iterator rit = queue.rbegin() ;
             rit != queue.rend() ; ++rit)
            XNextEvent(display_, &(*rit));

        logDebug << "queue: " << queue << std::endl;
        queue = filterQueue(queue);
        logDebug << "filtered: " << queue << "\n" << std::endl;

        for (size_t i = 0 ; i < queue.size() ; i++) {
            for (size_t j = 0 ; j < events_.size() ; j++) {
                events_[j].callBack(queue[i]);
                if (done())
                    return;
            }
        }
    }
}


void XWin::addEvent(int eventType, XWin *xwin, EventCallBack callBack){
    events_.push_back(Event(xwin, eventType, callBack));
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
