//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef XOSVIEW_H
#define XOSVIEW_H

#include "xwin.h"
#include "Xrm.h"  //  For Xrm resource manager class.

#include <string>



class Meter;
class XOSVFont;



class XOSView : public XWin {
public:
    XOSView(void);
    virtual ~XOSView(void);

    void run(int argc, char **argv);
    void scaffolding(int argc, char **argv);

    static double maxSampRate(void); // Samples/sec max
    std::string winname(void);

    // used by meter makers
    int xoff(void) const { return xoff_; }
    int newypos(void);

    bool hasBeenExposedAtLeastOnce() const { return exposed_once_flag_; }
    bool isExposed() const { return expose_flag_; }
    bool isFullyVisible() const { return _isvisible && !_ispartiallyvisible; }
    bool isAtLeastPartiallyVisible() const { return _isvisible; }

    //------------------------------------------------------
    // Resouce interface
    //------------------------------------------------------
    virtual std::string getResource(const std::string &name); // exit not found
    virtual std::string getResourceOrUseDefault(const std::string &name,
      const std::string &defaultVal);
    virtual bool isResourceTrue(const std::string &name);
    virtual void dumpResources(std::ostream &os);
    //------------------------------------------------------

protected:
    Xrm *_xrm;
    bool caption_, legend_, usedlabels_;
    int xoff_, yoff_;
    int hmargin_, vmargin_, vspacing_;
    unsigned long sleeptime_, usleeptime_;
    bool expose_flag_, exposed_once_flag_;
    bool _isvisible;
    bool _ispartiallyvisible;
    std::vector<Meter *> _meters;


    void reallydraw(void);
    void draw(void);
    virtual std::string className(void) { return _xrm->className(); }
    virtual std::string instanceName(void) { return _xrm->instanceName(); }
    void resize(void);
    void checkArgs(int argc, char** argv) const;
    void usleep_via_select(unsigned long usec);
    void checkMeterResources(void);
    void figureSize(void);
    int findx(XOSVFont &font);
    int findy(XOSVFont &font);
    void setSleepTime(void);
    void loadResources(int argc, char **argv);
    void setEvents(void);
    void createMeters(void);
    void checkVersion(int argc, char *argv[]) const;
    void dolegends(void);
    void checkOverallResources(void);
    void resizeEvent(XEvent &event);
    void exposeEvent(XExposeEvent &event);
    void keyPressEvent(XKeyEvent &event);
    void visibilityEvent(XVisibilityEvent &event);
    void unmapEvent(XUnmapEvent &event);

private:
    //  Take at most n samples per second (default of 10)
    static double MAX_SAMPLES_PER_SECOND;

    static std::string iname(int argc, char **argv);
};

#endif
