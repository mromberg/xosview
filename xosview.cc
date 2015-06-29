//
//  Copyright (c) 1994, 1995, 2002, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "xosview.h"
#include "meter.h"
#include "x11font.h"
#include "MeterMaker.h"
#include "strutil.h"
#if (defined(XOSVIEW_NETBSD) || defined(XOSVIEW_FREEBSD) || defined(XOSVIEW_OPENBSD))
#include "kernel.h"
#endif

#include <algorithm>

#include <unistd.h>



static const char * const versionString = "xosview version: " PACKAGE_VERSION;
static const char NAME[] = "xosview@";


double XOSView::MAX_SAMPLES_PER_SECOND = 10;


XOSView::XOSView(int argc, char *argv[])
    : XWin(), xrm(Xrm("xosview", iname(argc, argv))),
      expose_flag_(false), exposed_once_flag_(false) {

    // Check for version arguments first.  This allows
    // them to work without the need for a connection
    // to the X server
    checkVersion(argc, argv);

    setDisplayName(xrm.getDisplayName(argc, argv));

    openDisplay();  //  So that the Xrm class can contact the display for its
                    //  default values.

    //  The resources need to be initialized before calling XWinInit, because
    //  XWinInit looks at the geometry resource for its geometry.  BCG
    xrm.loadAndMergeResources(argc, argv, display());
    XWinInit (argc, argv);

    setSleepTime();

    loadResources(argc, argv); // General var init

    setEvents();  //  set up the X events

    checkOverallResources(); // see if legends are to be used

    createMeters(); // add in the meters

    // determine the width and height of the window then create it
    // These *HAVE* to come before the resource checking
    // because checking resources alloc colors
    // And there is no display set in the graphics
    // until init() does it's thing.
    figureSize();
    Xrm::opt geom = xrm.getResource("geometry");
    init(argc, argv, getResourceOrUseDefault("pixmapName", ""),
      geom.second, !geom.first);

    checkMeterResources(); //  Have the meters re-check the resources.

    title( winname() );
    iconname( winname() );
    dolegends();
    resize();
}

void XOSView::checkVersion(int argc, char *argv[]) const {
    for (int i = 0 ; i < argc ; i++)
        if ((util::tolower(argv[i]) == "-v")
          || (util::tolower(argv[i]) == "--version")) {
            std::cout << versionString << std::endl;
            exit(0);
        }
}

int XOSView::findx(X11Font &font){
    if ( legend_ ){
        if ( !usedlabels_ )
            return font.textWidth( "XXXXXXXXXXXXXXXXXXXXXXXX" );
        else
            return font.textWidth( "XXXXXXXXXXXXXXXXXXXXXXXXXXXXX" );
    }
    return 80;
}

int XOSView::findy(X11Font &font){
    if ( legend_ )
        return 10 + font.textHeight() * _meters.size() * ( caption_ ? 2 : 1 );

    return 15 * _meters.size();
}

void XOSView::figureSize(void) {
    std::string fname = getResource("font");
    X11Font font(display(), fname);
    if (!font)
        logFatal << "Could not load font: " << fname << std::endl;

    if ( legend_ ){
        if ( !usedlabels_ )
            xoff_ = font.textWidth( "XXXXX" );
        else
            xoff_ = font.textWidth( "XXXXXXXXX" );

        yoff_ = caption_ ? font.textHeight() + font.textHeight() / 4 : 0;
    }
    static bool firsttime = true;
    if (firsttime) {
        firsttime = false;
        width(findx(font));
        height(findy(font));
    }
}

void XOSView::checkMeterResources( void ){
    for (size_t i = 0 ; i < _meters.size() ; i++)
        _meters[i]->checkResources();
}

int XOSView::newypos( void ){
    return 15 + 25 * _meters.size();
}

void XOSView::dolegends( void ){
    for (size_t i = 0 ; i < _meters.size() ; i++) {
        _meters[i]->docaptions(caption_);
        _meters[i]->dolegends(legend_);
        _meters[i]->dousedlegends(usedlabels_);
    }
}

void XOSView::checkOverallResources() {
    //  Check various resource values.

    //  Set 'off' value.  This is not necessarily a default value --
    //    the value in the defaultXResourceString is the default value.
    usedlabels_ = legend_ = caption_ = 0;

    // use captions
    if ( isResourceTrue("captions") )
        caption_ = 1;

    // use labels
    if ( isResourceTrue("labels") )
        legend_ = 1;

    // use "free" labels
    if ( isResourceTrue("usedlabels") )
        usedlabels_ = 1;
}

std::string XOSView::winname( void ){
    char host[100];
    std::string hname("unknown");
    if (gethostname( host, 99 )) {
        logProblem << "gethostname() failed" << std::endl;
    }
    else {
        host[99] = '\0';  // POSIX.1-2001 says truncated names not terminated
        hname = std::string(host);
    }
    std::string name = std::string(NAME) + hname;
    return getResourceOrUseDefault("title", name);
}


void  XOSView::resize( void ){
    int spacing = vspacing_+1;
    int topmargin = vmargin_;
    int rightmargin = hmargin_;
    int newwidth = width() - xoff_ - rightmargin;
    size_t nmeters = _meters.size();
    nmeters = nmeters ? nmeters : 1; // don't divide by zero
    int newheight =
        (height() -
          (topmargin + topmargin + (nmeters-1)*spacing + nmeters*yoff_)
            ) / nmeters;
    newheight = (newheight >= 2) ? newheight : 2;

    for (size_t i = 0 ; i < _meters.size() ; i++) {
        _meters[i]->resize(xoff_,
          topmargin + (i + 1) * yoff_ + i * (newheight+spacing),
          newwidth, newheight);
    }
}


XOSView::~XOSView( void ){
    logDebug << "deleting " << _meters.size() << " meters..." << std::endl;
    for (size_t i = 0 ; i < _meters.size() ; i++)
        delete _meters[i];
    _meters.resize(0);
}

void XOSView::reallydraw( void ){
    logDebug << "Doing draw." << std::endl;
    g().clear();

    for (size_t i = 0 ; i < _meters.size() ; i++)
        _meters[i]->draw(g());

    g().flush();

    expose_flag_ = false;
}

void XOSView::draw ( void ) {
    if (hasBeenExposedAtLeastOnce() && isAtLeastPartiallyVisible())
        reallydraw();
    else {
        if (!hasBeenExposedAtLeastOnce()) {
            logDebug << "Skipping draw:  not yet exposed." << std::endl;
        } else if (!isAtLeastPartiallyVisible()) {
            logDebug << "Skipping draw:  not visible." << std::endl;
        }
    }
}

void XOSView::run( void ){
    int counter = 0;

    while( !done() ){
        checkevent();
        if (done()) {
            logDebug << "checkevent() set done" << std::endl;
            break; // XEvents can set this
        }

        if (_isvisible){
            for (size_t i = 0 ; i < _meters.size() ; i++) {
                if ( _meters[i]->requestevent() )
                    _meters[i]->checkevent();
            }

            g().flush();
        }
#ifdef HAVE_USLEEP
        /*  First, sleep for the proper integral number of seconds --
         *  usleep only deals with times less than 1 sec.  */
        if (sleeptime_)
            sleep((unsigned int)sleeptime_);
        if (usleeptime_)
            usleep( (unsigned int)usleeptime_);
#else
        usleep_via_select ( usleeptime_ );
#endif
        counter = (counter + 1) % 5;
    }
    logDebug << "leaging run()..." << std::endl;
}

void XOSView::usleep_via_select( unsigned long usec ){
    struct timeval time;

    time.tv_sec = (int)(usec / 1000000);
    time.tv_usec = usec - time.tv_sec * 1000000;

    select( 0, 0, 0, 0, &time );
}

void XOSView::keyPressEvent( XKeyEvent &event ){
    char c = 0;
    KeySym key;

    XLookupString( &event, &c, 1, &key, NULL );

    if ( (c == 'q') || (c == 'Q') )
        done(true);
}

void XOSView::checkArgs (int argc, char** argv) const {
    //  The XWin constructor call in the XOSView constructor above
    //  modifies argc and argv, so by this
    //  point, all XResource arguments should be removed.  Since we currently
    //  don't have any other command-line arguments, perform a check here
    //  to make sure we don't get any more.
    if (argc == 1)
        return;  //  No arguments besides X resources.

    //  Skip to the first real argument.
    argc--;
    argv++;
    while (argc > 0 && argv && *argv) {
        switch (argv[0][1]) {
        case 'n': //  Check for -name option that was already parsed
                  //  and acted upon by main().
            if (util::tolower(*argv) == "-name") {
                argv++;	//  Skip arg to -name.
                argc--;
            }
            break;
#if (defined(XOSVIEW_NETBSD) || defined(XOSVIEW_FREEBSD) || defined(XOSVIEW_OPENBSD))
        case 'N':
            if (strlen(argv[0]) > 2)
                SetKernelName(argv[0]+2);
            else {
                SetKernelName(argv[1]);
                argc--;
                argv++;
            }
            break;
#endif
            /*  Fall through to default/error case.  */
        default:
            logEvent << "Ignoring unknown option '" << argv[0] << "'.\n";
            break;
        }
        argc--;
        argv++;
    }
}

void XOSView::exposeEvent( XExposeEvent &event ) {
    _isvisible = true;
    if ( event.count == 0 ) {
        expose_flag_ = true;
        exposed_once_flag_ = true;
        draw();
    }
    logDebug << "Got expose event." << std::endl;
    if (!exposed_once_flag_) {
        exposed_once_flag_ = 1;
        draw();
    }
}

void XOSView::resizeEvent( XEvent &e ) {
    g().resize(e.xconfigure.width, e.xconfigure.height);
    resize();
    expose_flag_ = true;
    exposed_once_flag_ = true;
    draw();
}


void XOSView::visibilityEvent( XVisibilityEvent &event ){
    _ispartiallyvisible = false;
    if (event.state == VisibilityPartiallyObscured){
        _ispartiallyvisible = true;
    }

    if (event.state == VisibilityFullyObscured){
        _isvisible = false;
    }
    else {
        _isvisible = true;
    }
    logDebug << "Got visibility event; " << _ispartiallyvisible
             << " and " << _isvisible << std::endl;
}

void XOSView::unmapEvent( XUnmapEvent & ){
    _isvisible = false;
}

double XOSView::maxSampRate(void) {
    return MAX_SAMPLES_PER_SECOND;
}

std::string XOSView::iname(int, char **argv) {
    /*  Icky.  Need to check for -name option here.  */
    char** argp = argv;
    const char* instanceName = "xosview";	// Default value.
    while (argp && *argp) {
        if (std::string(*argp) == "-name")
            instanceName = argp[1];
        argp++;
    }  //  instanceName will end up pointing to the last such -name option.

    return instanceName;
}

void XOSView::setSleepTime(void) {
    MAX_SAMPLES_PER_SECOND = util::stof(getResource("samplesPerSec"));
    if (!MAX_SAMPLES_PER_SECOND)
        MAX_SAMPLES_PER_SECOND = 10;

    usleeptime_ = (unsigned long) (1000000/MAX_SAMPLES_PER_SECOND);
    if (usleeptime_ >= 1000000) {
        /*  The syscall usleep() only takes times less than 1 sec, so
         *  split into a sleep time and a usleep time if needed.  */
        sleeptime_ = usleeptime_ / 1000000;
        usleeptime_ = usleeptime_ % 1000000;
    } else {
        sleeptime_ = 0;
    }
#if (defined(XOSVIEW_NETBSD) || defined(XOSVIEW_FREEBSD) || defined(XOSVIEW_OPENBSD))
    BSDInit();	/*  Needs to be done before processing of -N option.  */
#endif
}

void XOSView::loadResources(int argc, char **argv) {
    hmargin_  = util::stoi(getResource("horizontalMargin"));
    vmargin_  = util::stoi(getResource("verticalMargin"));
    vspacing_ = util::stoi(getResource("verticalSpacing"));
    hmargin_  = std::max(0, hmargin_);
    vmargin_  = std::max(0, vmargin_);
    vspacing_ = std::max(0, vspacing_);

    checkArgs (argc, argv);  //  Check for any other unhandled args.
    xoff_ = hmargin_;
    yoff_ = 0;
    appName("xosview");
    _isvisible = false;
    _ispartiallyvisible = false;
    expose_flag_ = true;
    exposed_once_flag_ = false;
}

void XOSView::setEvents(void) {
    addEvent( new Event( this, ConfigureNotify,
        (EventCallBack)&XOSView::resizeEvent ) );
    addEvent( new Event( this, Expose,
        (EventCallBack)&XOSView::exposeEvent ) );
    addEvent( new Event( this, KeyPress,
        (EventCallBack)&XOSView::keyPressEvent ) );
    addEvent( new Event( this, VisibilityNotify,
        (EventCallBack)&XOSView::visibilityEvent ) );
    addEvent( new Event( this, UnmapNotify,
        (EventCallBack)&XOSView::unmapEvent ) );
}

void XOSView::createMeters(void) {
    MeterMaker mm(this);
    mm.makeMeters();
    for (int i = 1 ; i <= mm.n() ; i++)
        _meters.push_back(mm[i]);

    if (_meters.size() == 0)
        logProblem << "No meters were enabled." << std::endl;
}

bool XOSView::isResourceTrue( const std::string &name ) {
    Xrm::opt val = xrm.getResource(name);
    if (!val.first)
        return false;

    return val.second == "True";
}

std::string XOSView::getResourceOrUseDefault( const std::string &name,
  const std::string &defaultVal ){

    Xrm::opt retval = xrm.getResource (name);
    if (retval.first)
        return retval.second;

    return defaultVal;
}

std::string XOSView::getResource( const std::string &name ){
    Xrm::opt retval = xrm.getResource (name);
    if (retval.first)
        return retval.second;
    else {
        logFatal << "Couldn't find '" << name
                 << "' resource in the resource database!\n";
        /*  Some compilers aren't smart enough to know that exit() exits.  */
        return '\0';
    }
}

void XOSView::dumpResources( std::ostream &os ){
    xrm.dump(os);
}
