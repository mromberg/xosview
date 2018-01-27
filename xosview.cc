//
//  Copyright (c) 1994, 1995, 2002, 2006, 2015, 2016, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "xosview.h"
#include "meter.h"
#include "clopt.h"
#include "MeterMaker.h"
#include "strutil.h"
#include "fsutil.h"
#include "x11font.h"
#include "xsc.h"
#include "scache.h"
#include "x11graphics.h"
#ifdef HAVE_XFT
#include "xftfont.h"
#endif

#include <algorithm>

#include <sys/time.h>  //
#include <sys/types.h> // All three for select()
#include <unistd.h>    //



static const char * const VersionString = "xosview version: " PACKAGE_VERSION;
static const char * const NAME = "xosview@";



XOSView::XOSView(void)
    : XWin(), _xrm(0),
      _caption(false), _legend(false), _usedlabels(false),
      _xoff(0), _yoff(0),
      _hmargin(0), _vmargin(0), _vspacing(0),
      _sleeptime(1), _usleeptime(1000),
      _isvisible(false), _ispartiallyvisible(false), _sampleRate(10),
      _doFullDraw(true), _xsc(0) {
}


XOSView::~XOSView( void ){
    logDebug << "deleting " << _meters.size() << " meters..." << std::endl;
    for (size_t i = 0 ; i < _meters.size() ; i++)
        delete _meters[i];
    _meters.resize(0);
    delete _xrm;
    delete _xsc;
}


void XOSView::run(int argc, const char * const *argv) {

    // Convert argc and argv to STL containers.  No use of argc/argv
    // beyond this point.
    std::vector<std::string> vargv = util::vargv(argc, argv);
    logDebug << "cmdline args: " << vargv << std::endl;

    loadConfiguration(vargv);  // parse cmdline and open Xrm.
    openSession(vargv);        // X11R6+ session management.
    checkResources();          // initialize from our resources
    setEvents();               //  set up the X events
    createMeters();            // add in the meters
    figureSize();              // calculate size using number of meters
    createWindow();            // Graphics should now be up (can alloc colors)
    resdb().setGraphics(&g()); // So, ResDB can allocate colors
    resize();                  // initialize size values
    title(winname());          // Now that the window exists set the title
    iconname(winname());       // and the icon name
    checkMeterResources();     // Have the meters check their resources.

    loop(); // enter event loop
}


void XOSView::loop(void) {

    std::vector<Meter *> drawv;
    drawv.reserve(_meters.size());
    bool firstPass = true;  // checkevent and draw all meters on first pass.

    while( !done() ){
        // reset draw related vars.
        drawv.clear();
        _doFullDraw = false;

        // Check ICE/SM message once every 10 times.
        if (!(LoopCounter::count() % 10))
            done(_xsc->check());

        // If the session manager did not tell us to die check X.
        if (!done())
            checkevent();

        if (done()) {
            logDebug << "checkevent() set done" << std::endl;
            break; // XEvents or ICE/SM can set this.
        }

        if (_isvisible){
            for (size_t i = 0 ; i < _meters.size() ; i++) {
                if (_meters[i]->requestevent() || firstPass) {
                    _meters[i]->checkevent();
                    drawv.push_back(_meters[i]);
                }
            }
            if (firstPass)
                firstPass = false;
        }

        if (_doFullDraw)
            draw();
        else
            drawIfNeeded(drawv);

        swapBB();
        g().flush();
        slumber();
        LoopCounter::increment(); // invalidates cached stats.
    }
    logDebug << "leaving run()..." << std::endl;
}


void XOSView::createMeters(void) {
    MeterMaker mm;
    _meters = mm.makeMeters(resdb());

    if (_meters.size() == 0)
        logProblem << "No meters were enabled." << std::endl;

    dolegends(); // set global properties based on our resource
}


//-----------------------------------------------------------------
// ** Events
//-----------------------------------------------------------------

void XOSView::setEvents(void) {
    XWin::setEvents();

    addEvent( ConfigureNotify, this,
      (EventCallBack)&XOSView::configureEvent );
    addEvent( Expose, this, (EventCallBack)&XOSView::exposeEvent );
    addEvent( KeyPress, this, (EventCallBack)&XOSView::keyPressEvent );
    addEvent( VisibilityNotify, this,
      (EventCallBack)&XOSView::visibilityEvent );
    addEvent( UnmapNotify, this, (EventCallBack)&XOSView::unmapEvent );
}


void XOSView::keyPressEvent( XKeyEvent &event ){
    char c = 0;
    KeySym key;

    XLookupString( &event, &c, 1, &key, NULL );

    if ( (c == 'q') || (c == 'Q') )
        done(true);
}


void XOSView::exposeEvent( XExposeEvent &event ) {
    logDebug << "XOSView::exposeEvent(): count=" << event.count << std::endl;
    _isvisible = true;
    if ( event.count == 0 )
        scheduleDraw(true);
}


void XOSView::configureEvent( XEvent &e ) {
    unsigned int ew = e.xconfigure.width;
    unsigned int eh = e.xconfigure.height;
    logDebug << "configure event: " << ew << '/' << eh << std::endl;
    if ((g().width() != ew) || (g().height() != eh)) {
        logDebug << "XOSView::configureEvent(): set sizes..." << std::endl;
        g().resize(ew, eh);
        resize();
        //draw();
    }
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


void XOSView::drawIfNeeded(std::vector<Meter *> &mtrs) {
    if (isAtLeastPartiallyVisible()) {
        for (size_t i = 0 ; i < mtrs.size() ; i++)
            mtrs[i]->drawIfNeeded(g());
    }
}


void XOSView::draw ( void ) {
    if (isAtLeastPartiallyVisible()) {
        logDebug << "Doing full clear/draw." << std::endl;
        g().clear();

        for (size_t i = 0 ; i < _meters.size() ; i++)
            _meters[i]->draw(g());
    }
    else {
        logDebug << "********** FIXME ************\n";
        logDebug << "DRAW CALLED WHILE NOT VISIBLE\n";
        logDebug << "*****************************\n";
    }
}


void XOSView::usleep_via_select( unsigned long usec ){
    struct timeval time;

    time.tv_sec = (int)(usec / 1000000);
    time.tv_usec = usec - time.tv_sec * 1000000;

    select( 0, 0, 0, 0, &time );
}


void XOSView::slumber(void) const {
#ifdef HAVE_USLEEP
        /*  First, sleep for the proper integral number of seconds --
         *  usleep only deals with times less than 1 sec.  */
        if (_sleeptime)
            sleep((unsigned int)_sleeptime);
        if (_usleeptime)
            usleep( (unsigned int)_usleeptime);
#else
        usleep_via_select ( usleeptime_ );
#endif
}


void XOSView::openSession(const std::vector<std::string> &argv) {
    // Try to contact an X11R6 session manager...
    // sessionID (if it exists will be the old ID from the cmdline.
    _xsc = new XSessionClient(argv, "--smid",
      resdb().getResourceOrUseDefault("sessionID", ""));
    if (_xsc->init()) {
        logDebug << "session ID: " << _xsc->sessionID()
                 << std::endl;
    }

    // Set the sessionID in the resdb in case we have a new one.
    _xrm->putResource("." + instanceName() + "*sessionID",
      _xsc->sessionID());
}


//-----------------------------------------------------------------
// ** Configure
//-----------------------------------------------------------------

void XOSView::loadConfiguration(std::vector<std::string> &argv) {
    //...............................................
    // Command line options
    //...............................................
    if (!argv.empty())
        argv[0] = util::fs::findCommand(argv[0]); // full absolute path.

    util::CLOpts clopts(argv);
    setCommandLineArgs(clopts);
    clopts.parse(); // will exit() if fails.

    if (clopts.isTrue("help")) {
        std::cout << versionStr() << "\n\n" << clopts.useage() << std::endl;
        exit(0);
    }
    if (clopts.isTrue("version")) {
        std::cout << versionStr() << std::endl;
        exit(0);
    }

    //...............................................
    // X resources
    //...............................................
    _xrm = new Xrm(PACKAGE_CLASSNAME, clopts.value("name", "xosview"));

    //  Open the Display and load the X resources
    setDisplayName(clopts.value("displayName", ""));
    openDisplay();
    _xrm->loadResources(display());

    // Now load any resouce files specified on the command line
    const std::vector<std::string> &cfiles = clopts.values("configFile");
    for (size_t i = 0 ; i < cfiles.size() ; i++)
        if (!_xrm->loadResources(cfiles[i])) {
            logProblem << "Could not read file: " << cfiles[i]
                       << std::endl;
        }


    // load all of the command line options into the
    // resouce database.  First the ones speced by -xrm
    const std::vector<std::string> &xrml = clopts.values("xrm");
    for (size_t i = 0 ; i < xrml.size() ; i++) {
        _xrm->putResource(xrml[i]);
        logDebug << "ADD: " << xrml[i] << std::endl;
    }
    // And then those from -o
    const std::vector<std::string> &ol = clopts.values("xosvxrm");
    for (size_t i = 0 ; i < ol.size() ; i++) {
        std::string res(instanceName() + "*" + ol[i]);
        _xrm->putResource(res);
        logDebug << "ADD: " << res << std::endl;
    }

    // Now all the rest that are set by the user.
    // defaults dealt with by getResourceOrUseDefault()
    const std::vector<util::CLOpt> &opts = clopts.opts();
    for (size_t i = 0 ; i < opts.size() ; i++) {
        if (opts[i].name() != "xrm"
          && opts[i].name() != "xosvxrm"
          && !opts[i].missing()) {
            std::string rname("." + instanceName() + "*" +opts[i].name());
            _xrm->putResource(rname, opts[i].value());
            logDebug << "ADD: "
                     << rname << " : " << opts[i].value()
                     << std::endl;

#if defined(HAVE_LIB_SM)
            // Take the old session ID opt out of argv.
            if (opts[i].name() == "sessionID")
                opts[i].eraseFrom(argv);
#endif
        }
    }

    // The window manager will later wanna know the command line
    // arguments.  Since this may be used to restore a session, we
    // will save them here in a resource.
    logDebug << "modified cmdline args: " << argv << std::endl;
    _xrm->putResource("." + instanceName() + "*command",
      util::join(argv, " "));

    //---------------------------------------------------
    // No use of clopts beyond this point.  It is all in
    // the resource database now.  Example immediately follows...
    //---------------------------------------------------
    if (resdb().isResourceTrue("xrmdump")) {
        resdb().dump(std::cout);
        exit(0);
    }
}


void XOSView::checkResources(void) {
    setSleepTime();

    _hmargin  = util::stoi(resdb().getResource("horizontalMargin"));
    _vmargin  = util::stoi(resdb().getResource("verticalMargin"));
    _vspacing = util::stoi(resdb().getResource("verticalSpacing"));
    _hmargin  = std::max(0, _hmargin);
    _vmargin  = std::max(0, _vmargin);
    _vspacing = std::max(1, _vspacing);

    _xoff = _hmargin;
    _yoff = 0;
    appName("xosview");
    _isvisible = false;
    _ispartiallyvisible = false;

    //  Set 'off' value.  This is not necessarily a default value --
    //    the value in the defaultXResourceString is the default value.
    _usedlabels = _legend = _caption = false;

    // use captions
    if ( resdb().isResourceTrue("captions") )
        _caption = true;

    // use labels
    if ( resdb().isResourceTrue("labels") )
        _legend = true;

    // use "free" labels
    if ( resdb().isResourceTrue("usedlabels") )
        _usedlabels = true;
}


void XOSView::checkMeterResources( void ){
    for (size_t i = 0 ; i < _meters.size() ; i++)
        _meters[i]->checkResources(resdb());
}


int XOSView::newypos( void ){
    return 15 + 25 * _meters.size();
}


int XOSView::findx(XOSVFont &font){
    if ( _legend ){
        if ( !_usedlabels )
            return font.maxCharWidth() * 24;
        else
            return font.maxCharWidth() * 24
                + font.textWidth("SWAP 999% ");
    }
    return 80;
}


int XOSView::findy(void) {
    // Same match as in resize().
    const size_t nmeters = std::max(_meters.size(), (size_t)1);  // don't / by 0
    const int mh = 2 + _yoff + _vspacing;
    const int my = _vmargin + mh * nmeters;
    return my + 2 * _vmargin;
}


void XOSView::figureSize(void) {
    std::string fname = resdb().getResource("font");
    logDebug << "Font name: " << fname << std::endl;
#ifdef HAVE_XFT
    X11ftFont font(display(), fname);
#else
    X11Font font(display(), fname);
#endif
    if (!font)
        logFatal << "Could not load font: " << fname << std::endl;

    if ( _legend ){
        if ( !_usedlabels )
            _xoff = font.textWidth("INT(9) ");
        else
            _xoff = font.textWidth("SWAP 99%%");

        // The +6/+3 accounts for slop in the font drawing/clearing code.
        _yoff = _caption ? (font.textHeight() + 6): 6;
    }
    else
        _yoff = 3;

    static bool firsttime = true;
    if (firsttime) {
        firsttime = false;
        width(findx(font));
        height(findy());
        logDebug << "number of meters: " << _meters.size() << std::endl;
        logDebug << "text height: " << font.textHeight() << std::endl;
        logDebug << "Width/Height set to: " << width() << '/' << height()
                 << std::endl;
    }
}


std::string XOSView::versionStr(void) const {
    return VersionString;
}


void XOSView::setSleepTime(void) {
    _sampleRate = util::stof(resdb().getResource("samplesPerSec"));
    if (!_sampleRate)
        _sampleRate = 10;

    _usleeptime = (unsigned long) (1000000/_sampleRate);
    if (_usleeptime >= 1000000) {
        /*  The syscall usleep() only takes times less than 1 sec, so
         *  split into a sleep time and a usleep time if needed.  */
        _sleeptime = _usleeptime / 1000000;
        _usleeptime = _usleeptime % 1000000;
    }
    else {
        _sleeptime = 0;
    }
}


void XOSView::dolegends( void ){
    logDebug << "caption, legend, usedlabels: "
             << _caption << "," << _legend << "," << _usedlabels
             << std::endl;
    for (size_t i = 0 ; i < _meters.size() ; i++) {
        _meters[i]->docaptions(_caption);
        _meters[i]->dolegends(_legend);
        _meters[i]->dousedlegends(_usedlabels);
    }
}


std::string XOSView::winname( void ){
    char host[100];
    std::string hname("unknown");
    if (gethostname( host, 99 )) {
        logProblem << "gethostname() failed" << std::endl;
    }
    else {
        host[99] = '\0';  //POSIX.1-2001 says truncated names not terminated
        hname = std::string(host);
    }
    std::string name = std::string(NAME) + hname;
    return resdb().getResourceOrUseDefault("title", name);
}


void  XOSView::resize(void) {
    //-----------------------------------
    // Width
    //-----------------------------------
    unsigned int rightmargin = _hmargin;
    unsigned int xpad = _xoff + rightmargin; // reserved for padding.
    xpad = xpad > width() ? width() : xpad;  // clamp to width().

    int newwidth = width() - xpad;
    newwidth = (newwidth >= 2) ? newwidth : 2; // clamp to 2.

    //-----------------------------------
    // Height
    //-----------------------------------
    const size_t nmeters = std::max(_meters.size(), (size_t)1);  // don't / by 0
    const int minmh = 2 + _yoff + _vspacing;
    const int mh = std::max((int)((height() - _vmargin * 2) / nmeters), minmh);
    const int newheight = std::max(mh - _yoff - _vspacing, 2);

    logDebug << "-- meter height --\n"
             << "height() : " << height() << "\n"
             << "_vmargin : " << _vmargin << "\n"
             << "_vspacing: " << _vspacing << "\n"
             << "_yoff    : " << _yoff << "\n"
             << "minmh    : " << minmh << "\n"
             << "mh       : " << mh << "\n"
             << "newheight: " << newheight
             << std::endl;

    for (size_t i = 0 ; i < _meters.size() ; i++) {
        const int my = _vmargin + mh * (i + 1) - newheight - 2;
        _meters[i]->resize(_xoff, my, newwidth, newheight);
    }
}


void XOSView::setCommandLineArgs(util::CLOpts &o) {

    //------------------------------------------------------
    // Define ALL of the command line options here.
    //
    // Note.  Every command line option that is set by the user
    // will be loaded into the Xrm database using the name
    // given here.  So, you can lookup these options anywhere
    // getResource() is available.
    //------------------------------------------------------
    o.add("help",
      "-h", "--help",
      "Display this message and exit.");
    o.add("version",
      "-v", "--version",
      "Display version information and exit.");

    // General "common" X resouces
    //-----------------------------------------
    o.add("displayName",
      "-display", "--display", "name",
      "The name of the X display to connect to.");
    o.add("title",
      "-title", "--title", "title",
      "The title to use.");
    o.add("geometry",
      "-g", "-geometry", "geometry",
      "The X geometry string to use.");
    o.add("foreground",
      "-fg", "--foreground", "color",
      "The color to use for the foreground.");
    o.add("background",
      "-bg", "--background", "color",
      "The color to use for the background.");
    o.add("font",
      "-fn", "-font", "fontName",
      "The name of the font to use.");
    o.add("iconic",
      "-iconic", "--iconic",
      "Request to start in an iconic state.");
    o.add("name",
      "-name", "--name", "name",
      "The X resource instance name to use.");
    o.add("xrm",
      "-x", "-xrm",
      "spec",
      "Set an X resource using the supplied spec.  For example: "
      "-x 'xosview*background: red' would set the background to red.");
    o.add("xosvxrm",
      "-o", "--option",
      "spec",
      "Same as -xrm but prepends the string 'xosview*' (insance name) "
      "for you.  So: -o 'background: red' would set the background "
      "to red.");

    // Xosview specific options
    //----------------------------------------------------
    o.add("configFile",
      "-c", "--config", "fileName",
      "Load an XResource file containing overrides.");
    o.add("xrmdump",
      "-xrmd", "--xrm-dump",
      "Dump the X resouces seen by xosview to stdout and exit.");

#if defined(HAVE_LIB_SM)
    // X Session Managment ID.
    o.add("sessionID",
      "-smid", "--smid", "sessionID",
      "Session management ID.");
#endif

    //-----------------------------------------------------
    // No other options that override X resources are needed
    // as they can be easily done with one or more of the
    // methods above.  We don't need to support as many options
    // as the UNIX ls command.
    //-----------------------------------------------------
}
