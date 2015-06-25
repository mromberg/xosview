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
class X11Font;

class XOSView : public XWin {
public:
    XOSView(int argc, char *argv[]);
    ~XOSView(void);

    void run(void);

    static double maxSampRate(void); // Samples/sec max

    void resize( void );
    void reallydraw( void );
    void draw ( void );


    std::string winname( void );

    // used by meter makers
    int xoff(void) const { return xoff_; }
    int newypos( void );

    int hasBeenExposedAtLeastOnce() const {return exposed_once_flag_; }
    int isExposed() const { return expose_flag_; }
    int isFullyVisible() const { return _isvisible && !_ispartiallyvisible; }
    int isAtLeastPartiallyVisible() const { return _isvisible; }

protected:

    Xrm xrm;
    void checkArgs (int argc, char** argv) const;
    class MeterNode {
    public:
        MeterNode( Meter *fm ) { meter_ = fm;  next_ = NULL; }

        Meter *meter_;
        MeterNode *next_;
    };

    MeterNode *meters_;

    int caption_, legend_, xoff_, yoff_, nummeters_, usedlabels_;
    int hmargin_, vmargin_, vspacing_;
    unsigned long sleeptime_, usleeptime_;

    int expose_flag_, exposed_once_flag_;

    void usleep_via_select( unsigned long usec );
    void addmeter( Meter *fm );
    void checkMeterResources( void );

private:
    void figureSize(void);
    int findx(X11Font &font);
    int findy(X11Font &font);
protected:
    void dolegends( void );

    void checkOverallResources();
    void resizeEvent( XEvent & );
    void exposeEvent( XExposeEvent &event );
    void keyPressEvent( XKeyEvent &event );
    void visibilityEvent( XVisibilityEvent &event );
    void unmapEvent( XUnmapEvent &event);
    void checkVersion(int argc, char *argv[]) const;

private:

    bool _isvisible;
    bool _ispartiallyvisible;

    //  Take at most n samples per second (default of 10)
    static double MAX_SAMPLES_PER_SECOND;

    static std::string iname(int argc, char **argv);

    void setSleepTime(void);
    void loadResources(int argc, char **argv);
    void setEvents(void);
    void createMeters(void);
};

#endif
