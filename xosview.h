//  
//  Copyright (c) 1994, 1995 by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: xosview.h,v 1.10 1999/02/26 23:28:13 bgrayson Exp $
//
#ifndef _XOSVIEW_H_
#define _XOSVIEW_H_

#define XOSVIEW_H_CVSID	"$Id: xosview.h,v 1.10 1999/02/26 23:28:13 bgrayson Exp $"

#include "xwin.h"
#include "Xrm.h"  //  For Xrm resource manager class.


/*  Take at most n samples per second (default of 10)  */
extern double MAX_SAMPLES_PER_SECOND;

class Meter;

class XOSView : public XWin {
public:
  XOSView( char* instName, int argc, char *argv[] );
  ~XOSView( void );

  void figureSize ( void );
  void resize( void );
  void draw( void );
  void safedraw ( void );
  void run( void );
  void keyrelease( char *ch );
  const char *winname( void );

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
private:

  bool _isvisible;
  bool _ispartiallyvisible;
};

/*  Make XOSDEBUG("This is a format string.  %d %d\n", a, b); look
 *  like if (0) printf("This ..."...);.  Change the 0 to a 1, to
 *  enable these debugging outputs.  */
#define XOSDEBUG	if (0) printf
#endif
