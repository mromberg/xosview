//
//  Copyright (c) 1994, 1995, 2006, 2015, 2016
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef XOSVIEW_H
#define XOSVIEW_H

#include "xwin.h"
#include "Xrm.h"

#include <string>


class Meter;
class XOSVFont;
class XSessionClient;
namespace util { class CLOpts; }


class XOSView : private XWin {
public:
    XOSView(void);
    virtual ~XOSView(void);

    void run(int argc, const char * const *argv);

protected:
    virtual ResDB &resdb(void) { return *_xrm; }
    virtual void setEvents(void);

private:
    Xrm *_xrm;
    bool _caption, _legend, _usedlabels;
    int _xoff, _yoff;
    int _hmargin, _vmargin, _vspacing;
    unsigned long _sleeptime, _usleeptime;
    bool _isvisible;
    bool _ispartiallyvisible;
    std::vector<Meter *> _meters;
    double _sampleRate;   // samples/sec
    bool _doFullDraw;     // schedule full clear/draw
    XSessionClient *_xsc; // session management client.

    double sampleRate(void) const { return _sampleRate; } // samples/sec max
    std::string winname(void);
    int xoff(void) const { return _xoff; }
    int newypos(void);
    bool isFullyVisible() const { return _isvisible && !_ispartiallyvisible; }
    bool isAtLeastPartiallyVisible() const { return _isvisible; }
    std::string versionStr(void) const;
    void loop(void);
    void loadConfiguration(const std::vector<std::string> &argv);
    void setCommandLineArgs(util::CLOpts &o);
    void draw(void);
    void drawIfNeeded(std::vector<Meter *> &mtrs);
    std::string className(void) { return _xrm->className(); }
    std::string instanceName(void) { return _xrm->instanceName(); }
    void resize(void);
    void checkMeterResources(void);
    void figureSize(void);
    int findx(XOSVFont &font);
    int findy(XOSVFont &font);
    void setSleepTime(void);
    void checkResources(void);
    void createMeters(void);
    void dolegends(void);
    void configureEvent(XEvent &event);
    void exposeEvent(XExposeEvent &event);
    void keyPressEvent(XKeyEvent &event);
    void visibilityEvent(XVisibilityEvent &event);
    void unmapEvent(XUnmapEvent &event);
    void scheduleDraw(bool full) { _doFullDraw = full; }
    void slumber(void) const;
    void usleep_via_select(unsigned long usec);
};

#endif
