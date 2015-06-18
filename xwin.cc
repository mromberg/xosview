#include <X11/Xatom.h>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>
#include "general.h"
#include "xwin.h"
#include "Xrm.h"
#include "log.h"


//-----------------------------------------------------------------------------
//  argc is a reference, so that the changes to argc by XrmParseCommand are
//  noticed by the caller (XOSView, in this case).  BCG
XWin::XWin() {
}

XWin::XWin( int argc, char *argv[], int x, int y, int width, int height ) {

    logFatal << "This constructor call is not supported! (" << __FILE__
             << ":" << __LINE__ << ")" << std::endl;
    //  FIXME BCG  This constructor needs to do much of the work of the above
    //  one.  Or, we need to drop this as a constructor.  As it is, it is
    //  VERY MUCH out of date.
    x_ = x;
    y_ = y;
    width_ = width;
    height_ = height;
    (void) argc;
    (void) argv;
}


void XWin::XWinInit (int argc, char** argv, char* geometry, Xrm* xrm) {
    (void) argc;
    (void) argv;  //  Avoid gcc warnings about unused variables.
    //  Eventually, we may want to have XWin handle some arguments other
    //  than resources, so argc and argv are left as parameters.  BCG

    geometry_ = geometry;  //  Save for later use.
    width_ = height_ = x_ = y_ = 0;
    xrmptr_ = xrm;

    font_ = NULL;
    done_ = 0;

    // Set up the default Events
    events_ = NULL;
    addEvent( new Event( this, ConfigureNotify, &XWin::configureEvent ) );
    addEvent( new Event( this, ClientMessage, &XWin::deleteEvent ) );
    addEvent( new Event( this, MappingNotify, &XWin::mappingNotify ) );

    //openDisplay();  //  Done explicitly in xosview.cc.
}

XWin::~XWin( void ){

    // delete the events
    Event *event = events_;
    while ( event != NULL ){
        Event *save = event->next_;
        delete event;
        event = save;
    }

    XFree( title_.value );
    XFree( iconname_.value );
    XFree( sizehints_ );
    XFreeGC( display_, gc_ );
    XFreeFont( display_, font_ );
    XDestroyWindow( display_, window_ );
    // close the connection to the display
    XCloseDisplay( display_ );
}


void XWin::init( int argc, char **argv ){
    XGCValues            gcv;
    XSetWindowAttributes xswa;
    Pixmap	       background_pixmap;
    int		       doPixmap = 0;

    setFont();
    setColors();
    getGeometry();
#ifdef HAVE_XPM
    doPixmap=getPixmap(&background_pixmap);
#endif

    window_ = XCreateSimpleWindow(display_, DefaultRootWindow(display_),
      sizehints_->x, sizehints_->y,
      sizehints_->width, sizehints_->height,
      1,
      fgcolor_, bgcolor_);

    setHints( argc, argv );

    // Create a graphics context for the main window
    gcv.font = font_->fid;
    gcv.foreground = fgcolor_;
    gcv.background = bgcolor_;
    gc_ = XCreateGC(display_, window_,
      (GCFont | GCForeground | GCBackground), &gcv);

    // Set main window's attributes (colormap, bit_gravity)
    xswa.colormap = colormap_;
    xswa.bit_gravity = NorthWestGravity;
    XChangeWindowAttributes(display_, window_,
      (CWColormap | CWBitGravity), &xswa);

    // If there is a pixmap file, set it as the background
    if(doPixmap) {
	XSetWindowBackgroundPixmap(display_,window_,background_pixmap);
    }

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

    // Map the main window
    map();
    flush();
    if(XGetWindowAttributes(display_, window_, &attr_) == 0){
        logFatal << "Error getting attributes of Main." << std::endl;
    }

    //  Create stipple pixmaps.
    stipples_[0] = createPixmap("\000\000", 2, 2);
    stipples_[1] = createPixmap("\002\000\001\000", 2, 4);
    stipples_[2] = createPixmap("\002\001", 2, 2);
    stipples_[3] = createPixmap("\002\003\001\003", 2, 4);
    doStippling_ = isResourceTrue("enableStipple");
}

void XWin::setFont( void ){
    // Set up the font
    if ( font_ != NULL )
        return;
    std::string fontName = getResource("font");

    if ((font_ = XLoadQueryFont(display_, fontName.c_str())) == NULL){
        logFatal << name_ << ": display " << DisplayString(display_)
                 << " cannot load font " << fontName << std::endl;
    }
}

void XWin::setHints( int argc, char *argv[] ){
    // Set up class hint
    XClassHint    *classhints;   //  Class hint for window manager
    if((classhints = XAllocClassHint()) == NULL){
        logFatal << "Error allocating class hint!" << std::endl;
    }
    //  We have to cast away the const's.
    std::string cname = xrmptr_->className();
    std::string iname = xrmptr_->instanceName();
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
    if(XStringListToTextProperty(&np, 1, &title_) == 0){
        logFatal << "Error creating XTextProperty!" << std::endl;
    }
    if(XStringListToTextProperty(&np, 1, &iconname_) == 0){
        logFatal << "Error creating XTextProperty!" << std::endl;
    }

    XSetWMProperties(display_, window_, &title_, &iconname_, argv, argc,
      sizehints_, wmhints, classhints);

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

int XWin::getPixmap(Pixmap *pixmap) {
#ifdef HAVE_XPM
    char	*pixmap_file;
    XWindowAttributes    root_att;
    XpmAttributes        pixmap_att;

    pixmap_file = (char*) getResourceOrUseDefault("pixmapName",NULL);

    if (pixmap_file) {
        XGetWindowAttributes(display_, DefaultRootWindow(display_),&root_att);
        pixmap_att.closeness=30000;
        pixmap_att.colormap=root_att.colormap;
        pixmap_att.valuemask=XpmSize|XpmReturnPixels|XpmColormap|XpmCloseness;
        if(XpmReadFileToPixmap(display_,DefaultRootWindow(display_),
            pixmap_file, pixmap, NULL, &pixmap_att)) {
            logProblem << "Pixmap " << pixmap_file  << " not found"
                       << std::endl
                       << "Defaulting to blank" << std::endl;
            pixmap=NULL;
            return 0; // OOps
        }
        return 1;  // Good, found the pixmap
    }
    return 0; // No file specified, none used
#else
    (void) pixmap;
    logBug << "getPixmap called, when Xpm is not enabled!\n" ;
    return 0;
#endif
}

void XWin::getGeometry( void ){
    int                  bitmask;

    // Fill out a XsizeHints structure to inform the window manager
    // of desired size and location of main window.
    if((sizehints_ = XAllocSizeHints()) == NULL){
        logFatal << "Error allocating size hints!" << std::endl;
    }
    sizehints_->flags = PSize;
    sizehints_->height = height_;
    sizehints_->min_height = sizehints_->height;
    sizehints_->width = width_;
    sizehints_->min_width = sizehints_->width;
    sizehints_->x = x_;
    sizehints_->y = y_;

    // Construct a default geometry string
    std::ostringstream defgs;
    defgs << sizehints_->width << "x" << sizehints_->height << "+"
          << sizehints_->x << "+" << sizehints_->y;

    // Process the geometry specification
    Xrm::opt gopt = xrmptr_->getResource("geometry");
    const char *gptr = geometry_;
    if (gopt.first)
        gptr = gopt.second.c_str();
    bitmask =  XGeometry(display_, DefaultScreen(display_), gptr,
      defgs.str().c_str(),
      0,
      1, 1, 0, 0, &(sizehints_->x), &(sizehints_->y),
      &(sizehints_->width), &(sizehints_->height));

    // Check bitmask and set flags in XSizeHints structure
    if (bitmask & (WidthValue | HeightValue)){
        sizehints_->flags |= PPosition;
        width_ = sizehints_->width;
        height_ = sizehints_->height;
    }

    if (bitmask & (XValue | YValue)){
        sizehints_->flags |= USPosition;
        x_ = sizehints_->x;
        y_ = sizehints_->y;
    }
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

    while ( XEventsQueued( display_, QueuedAfterReading ) ){
        XNextEvent( display_, &event );

        // call all of the Event's call back functions to process this event
        Event *tmp = events_;
        while ( tmp != NULL ){
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

bool XWin::isResourceTrue( const std::string &name ) {
    Xrm::opt val = xrmptr_->getResource(name);
    if (!val.first)
        return false;

    return val.second == "True";
}

std::string XWin::getResourceOrUseDefault( const std::string &name,
  const std::string &defaultVal ){

    Xrm::opt retval = xrmptr_->getResource (name);
    if (retval.first)
        return retval.second;

    return defaultVal;
}

std::string XWin::getResource( const std::string &name ){
    Xrm::opt retval = xrmptr_->getResource (name);
    if (retval.first)
        return retval.second;
    else {
        logFatal << "Couldn't find '" << name
                 << "' resource in the resource database!\n";
        /*  Some compilers aren't smart enough to know that exit() exits.  */
        return '\0';
    }
}

void XWin::dumpResources( std::ostream &os ){
    logProblem << "Function not implemented!\n";  //BCG FIXME Need to make this.
    (void) os;  //  Keep gcc happy.
}

unsigned long XWin::allocColor( const std::string &name ){
    XColor exact, closest;

    if ( XAllocNamedColor( display_, colormap(), name.c_str(), &closest,
        &exact ) == 0 )
        logProblem << "XWin::allocColor() : failed to alloc : "
                   << name << std::endl;

    return exact.pixel;
}

void XWin::configureEvent( XEvent &event ){
    x( event.xconfigure.x );
    y( event.xconfigure.y );
    width( event.xconfigure.width );
    height( event.xconfigure.height );
}

void XWin::deleteEvent( XEvent &event ){
    if ( (event.xclient.message_type == wm_ ) &&
      ((unsigned)event.xclient.data.l[0] == wmdelete_) )
        done( 1 );
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
