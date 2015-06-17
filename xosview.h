//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef _XOSVIEW_H_
#define _XOSVIEW_H_

#include <string>
#include "xwin.h"
#include "Xrm.h"  //  For Xrm resource manager class.
#include "log.h"


/*  Take at most n samples per second (default of 10)  */
extern double MAX_SAMPLES_PER_SECOND;

class Meter;

class XOSView : public XWin {
public:
    XOSView( const std::string &instName, int argc, char *argv[] );
    ~XOSView( void );

    void figureSize ( void );
    void resize( void );
    void reallydraw( void );
    void draw ( void );
    void run( void );

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

    int findx( void );
    int findy( void );
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
};

#endif
