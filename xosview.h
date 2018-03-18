//
//  Copyright (c) 1994, 1995, 2006, 2015, 2016, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef xosview_h
#define xosview_h

#include "xwin.h"

#include <memory>

class Meter;
class XOSVFont;
class XSessionClient;
class ResDB;
class Xrm;
namespace util { class CLOpts; }



class XOSView : private XWin {
public:
    XOSView(void);
    virtual ~XOSView(void);

    void run(int argc, const char * const *argv);

protected:
    virtual ResDB &resdb(void) const override;
    virtual void setEvents(void) override;

private:
    std::unique_ptr<Xrm> _xrm;
    bool _caption, _legend, _usedlabels;
    int _xoff, _yoff;
    int _hmargin, _vmargin, _vspacing;
    unsigned long _sleeptime, _usleeptime;
    bool _isvisible;
    std::vector<std::unique_ptr<Meter>> _meters;
    double _sampleRate;   // samples/sec
    bool _doFullDraw;     // schedule full clear/draw
    std::unique_ptr<XSessionClient> _xsc; // session management client.

    std::string winname(void);
    int newypos(void);
    std::string versionStr(void) const;
    void loop(void);
    void loadConfiguration(std::vector<std::string> &argv);
    void openSession(const std::vector<std::string> &argv);
    void setCommandLineArgs(util::CLOpts &o);
    void draw(void);
    void drawIfNeeded(std::vector<Meter *> &mtrs);
    std::string className(void) const;
    std::string instanceName(void) const;
    void resize(void);
    void checkMeterResources(void);
    void figureSize(void);
    int findx(XOSVFont &font);
    int findy(void);
    void setSleepTime(void);
    void checkResources(void);
    void createMeters(void);
    void dolegends(void);
    void scheduleDraw(bool full) { _doFullDraw = full; }
    void slumber(void) const;
    void slumberOld(void) const;
    void usleep_via_select(unsigned long usec) const;
    void configureEvent(const XEvent &event);
    void exposeEvent(const XEvent &event);
    void keyPressEvent(const XEvent &event);
    void visibilityEvent(const XEvent &event);
    void unmapEvent(const XEvent &event);
};

#endif
