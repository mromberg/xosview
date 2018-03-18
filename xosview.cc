//
//  Copyright (c) 1994, 1995, 2002, 2006, 2015, 2016, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "xosview.h"
#include "Xrm.h"
#include "meter.h"
#include "clopt.h"
#include "MeterMaker.h"
#include "fsutil.h"
#include "strutil.h"
#include "x11font.h"
#include "xsc.h"
#include "scache.h"
#include "x11graphics.h"
#include "log.h"
#ifdef HAVE_XFT
#include "xftfont.h"
#endif

#include <array>
#include <thread>  // for std::this_thread::sleep_for() only.

#include <sys/time.h>  //
#include <sys/types.h> // All three for select()
#include <unistd.h>    //



static const char * const VersionString = "xosview version: " PACKAGE_VERSION;
static const char * const NAME = "xosview@";



XOSView::XOSView(void)
    : XWin(),
      _caption(false), _legend(false), _usedlabels(false),
      _xoff(0), _yoff(0),
      _hmargin(0), _vmargin(0), _vspacing(0),
      _sleeptime(1), _usleeptime(1000),
      _isvisible(false), _sampleRate(10),
      _doFullDraw(true) {
}


XOSView::~XOSView(void) = default;


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

    while(!done()) {
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

        if (_isvisible) {
            for (const auto &meter : _meters) {
                if (meter->requestevent() || firstPass) {
                    meter->checkevent();
                    drawv.push_back(meter.get());
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

    if (_meters.empty())
        logProblem << "No meters were enabled." << std::endl;

    dolegends(); // set global properties based on our resource
}


//-----------------------------------------------------------------
// Events
//-----------------------------------------------------------------

void XOSView::setEvents(void) {
    XWin::setEvents();

    // (the this->METHOD() thing is a gcc bug workaround.
    addEvent(ConfigureNotify, [this](auto e){ this->configureEvent(e); });
    addEvent(Expose, [this](auto e){ this->exposeEvent(e); });
    addEvent(KeyPress, [this](auto e){ this->keyPressEvent(e); });
    addEvent(VisibilityNotify, [this](auto e){ this->visibilityEvent(e); });
    addEvent(UnmapNotify, [this](auto e){ this->unmapEvent(e); });
}


void XOSView::keyPressEvent(const XEvent &e) {
    char c = 0;
    KeySym key;

    XLookupString(const_cast<XKeyEvent *>(&e.xkey), &c, 1, &key, nullptr);

    if ((c == 'q') || (c == 'Q'))
        done(true);
}


void XOSView::exposeEvent(const XEvent &e) {
    logDebug << "XOSView::exposeEvent(): count=" << e.xexpose.count
             << std::endl;
    _isvisible = true;
    if (e.xexpose.count == 0)
        scheduleDraw(true);
}


void XOSView::configureEvent(const XEvent &e) {

    const unsigned int ew = e.xconfigure.width;
    const unsigned int eh = e.xconfigure.height;
    logDebug << "configure event: " << ew << '/' << eh << std::endl;

    if ((g().width() != ew) || (g().height() != eh)) {
        logDebug << "XOSView::configureEvent(): set sizes..." << std::endl;
        g().resize(ew, eh);
        resize();
    }
}


void XOSView::visibilityEvent(const XEvent &e) {

    _isvisible = e.xvisibility.state != VisibilityFullyObscured;

    logDebug << "Got visibility event: " << _isvisible << std::endl;
}


void XOSView::unmapEvent(const XEvent &) {
    _isvisible = false;
}


void XOSView::drawIfNeeded(std::vector<Meter *> &mtrs) {
    if (_isvisible)
        for (auto &mtr : mtrs)
            mtr->drawIfNeeded(g());
}


void XOSView::draw(void) {
    if (_isvisible) {
        logDebug << "Doing full clear/draw." << std::endl;
        g().clear();

        for (auto &meter : _meters)
            meter->draw(g());
    }
    else {
        logDebug << "********** FIXME ************\n"
                 << "DRAW CALLED WHILE NOT VISIBLE\n"
                 << "*****************************" << std::endl;;
    }
}


void XOSView::usleep_via_select(unsigned long usec) {
    struct timeval time;

    time.tv_sec = static_cast<int>(usec / 1000000);
    time.tv_usec = usec - time.tv_sec * 1000000;

    select(0, 0, 0, 0, &time);
}


void XOSView::slumber(void) const {
    std::this_thread::sleep_for(std::chrono::microseconds(_usleeptime));
}


void XOSView::slumberOld(void) const {
#ifdef HAVE_USLEEP
    //  First, sleep for the proper integral number of seconds --
    //  usleep only deals with times less than 1 sec.
    if (_sleeptime)
        sleep(static_cast<unsigned int>(_sleeptime));
    if (_usleeptime)
        usleep(static_cast<unsigned int>(_usleeptime));
#else
    usleep_via_select(usleeptime_);
#endif
}


void XOSView::openSession(const std::vector<std::string> &argv) {
    // Try to contact an X11R6 session manager...
    // sessionID (if it exists will be the old ID from the cmdline.
    _xsc = std::make_unique<XSessionClient>(argv, "--smid",
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
    _xrm = std::make_unique<Xrm>(PACKAGE_CLASSNAME,
      clopts.value("name", "xosview"));

    //  Open the Display and load the X resources
    setDisplayName(clopts.value("displayName", ""));
    openDisplay();
    _xrm->loadResources(display());

    // Now load any resouce files specified on the command line
    for (const auto &cfile : clopts.values("configFile"))
        if (!_xrm->loadResources(cfile)) {
            logProblem << "Could not read file: " << cfile << std::endl;
        }

    // load all of the command line options into the
    // resouce database.  First the ones speced by -xrm
    for (const auto &xrm : clopts.values("xrm")) {
        _xrm->putResource(xrm);
        logDebug << "ADD: " << xrm << std::endl;
    }
    // And then those from -o
    for (const auto &o : clopts.values("xosvxrm")) {
        const std::string res(instanceName() + "*" + o);
        _xrm->putResource(res);
        logDebug << "ADD: " << res << std::endl;
    }

    // Now all the rest that are set by the user.
    // defaults dealt with by getResourceOrUseDefault()
    for (const auto &opt : clopts.opts()) {
        if (opt.name() != "xrm" && opt.name() != "xosvxrm"
          && !opt.missing()) {
            const std::string rname("." + instanceName() + "*" + opt.name());
            _xrm->putResource(rname, opt.value());
            logDebug << "ADD: "
                     << rname << " : " << opt.value()
                     << std::endl;

#if defined(HAVE_LIB_SM)
            // Take the old session ID opt out of argv.
            if (opt.name() == "sessionID")
                opt.eraseFrom(argv);
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

    _hmargin  = std::stoi(resdb().getResource("horizontalMargin"));
    _vmargin  = std::stoi(resdb().getResource("verticalMargin"));
    _vspacing = std::stoi(resdb().getResource("verticalSpacing"));
    _hmargin  = std::max(0, _hmargin);
    _vmargin  = std::max(0, _vmargin);
    _vspacing = std::max(1, _vspacing);

    _xoff = _hmargin;
    _yoff = 0;
    appName("xosview");
    _isvisible = false;

    //  Set 'off' value.  This is not necessarily a default value --
    //    the value in the defaultXResourceString is the default value.
    _usedlabels = _legend = _caption = false;

    // use captions
    if (resdb().isResourceTrue("captions"))
        _caption = true;

    // use labels
    if (resdb().isResourceTrue("labels"))
        _legend = true;

    // use "free" labels
    if (resdb().isResourceTrue("usedlabels"))
        _usedlabels = true;
}


void XOSView::checkMeterResources(void) {
    for (auto &meter : _meters)
        meter->checkResources(resdb());
}


int XOSView::newypos(void) {
    return 15 + 25 * _meters.size();
}


int XOSView::findx(XOSVFont &font) {
    if (_legend) {
        if (!_usedlabels)
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

    if (_legend) {
        if (!_usedlabels)
            _xoff = font.textWidth("INT(9) ");
        else
            _xoff = font.textWidth("SWAP 99%%");

        // The +6/+3 accounts for slop in the font drawing/clearing code.
        _yoff = _caption ? (font.textHeight() + 6) : 6;
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
    _sampleRate = std::stof(resdb().getResource("samplesPerSec"));
    if (!_sampleRate)
        _sampleRate = 10;

    _usleeptime = (unsigned long) (1000000/_sampleRate);
    if (_usleeptime >= 1000000) {
        //  The syscall usleep() only takes times less than 1 sec, so
        //  split into a sleep time and a usleep time if needed.
        _sleeptime = _usleeptime / 1000000;
        _usleeptime = _usleeptime % 1000000;
    }
    else
        _sleeptime = 0;
}


void XOSView::dolegends(void) {
    logDebug << "caption, legend, usedlabels: "
             << _caption << "," << _legend << "," << _usedlabels
             << std::endl;
    for (auto &meter : _meters) {
        meter->docaptions(_caption);
        meter->dolegends(_legend);
        meter->dousedlegends(_usedlabels);
    }
}


std::string XOSView::winname(void) {
    std::array<char, 100> host;
    std::string hname("unknown");
    if (gethostname(host.data(), host.size())) {
        logProblem << "gethostname() failed" << std::endl;
    }
    else {
        host.back() = '\0';  //POSIX.1-2001 says truncated names not terminated.
        hname = std::string(host.data());
    }

    return resdb().getResourceOrUseDefault("title", NAME + hname);
}


void  XOSView::resize(void) {
    //-----------------------------------
    // Width
    //-----------------------------------
    const unsigned int rightmargin = _hmargin;
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


ResDB &XOSView::resdb(void) const {
    return *_xrm;
}


std::string XOSView::className(void) const {
    return _xrm->className();
}


std::string XOSView::instanceName(void) const {
    return _xrm->instanceName();
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
