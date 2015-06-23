#ifndef _XWIN_H_
#define _XWIN_H_

#include "Xrm.h"
#include "x11graphics.h"

#include <iostream>
#include <string>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#ifdef HAVE_XPM
#include <X11/xpm.h>
#endif



class XWin;
class X11Pixmap;

typedef void (XWin::*EventCallBack)( XEvent &event );


class XWin {
public:
    XWin ();
    virtual ~XWin( void );
    void XWinInit ( int argc, char* argv[], char* geometry, Xrm* xrmp );

    int x( void ) { return x_; }
    void x( int val ) { x_ = val; }
    int y( void ) { return y_; }
    void y( int val ) { y_ = val; }
    int width( void ) { return width_; }
    void width( int val ) { width_ = val; }
    int height( void ) { return height_; }
    void height( int val ) { height_ = val; }
    Display *display( void ) { return display_; }
    Window window( void ) { return window_; }
    int done( void ) { return done_; }
    void done( int val ) { done_ = val; }
    void title( const std::string &str )
        { XStoreName( display_, window_, str.c_str() ); }
    void iconname( const std::string &str )
        { XSetIconName( display_, window_, str.c_str() ); }

    //-----------------------------------------------
    // New Graphics interface (in progress)
    //-----------------------------------------------
    X11Graphics &g(void) { return *_graphics; }
    X11Pixmap *newX11Pixmap(unsigned int width, unsigned int height);
    //-----------------------------------------------

    void clear( void ) { XClearWindow( display_, window_ ); }
    void clear( int x, int y, int width, int height )
        { XClearArea( display_, window_, x, y, width, height, False ); }
    unsigned long allocColor( const std::string &name );
    void setForeground( unsigned long pixelvalue )
        { XSetForeground( display_, gc_, pixelvalue ); }
    void setBackground( unsigned long pixelvalue )
        { XSetBackground( display_, gc_, pixelvalue ); }
    void setStipple( Pixmap stipple);
    void setStippleN (int n) {setStipple(stipples_[n]); }
    Pixmap createPixmap(const char* data, unsigned int w, unsigned int h);

    unsigned long foreground( void ) { return fgcolor_; }
    unsigned long background( void ) { return bgcolor_; }
    void resize( int width, int height )
        { XResizeWindow( display_, window_, width, height ); }
    void lineWidth( int width );
    void drawLine( int x1, int y1, int x2, int y2 )
        { XDrawLine( display_, window_, gc_, x1, y1, x2, y2 ); }
    void drawRectangle( int x, int y, int width, int height )
        { XDrawRectangle( display_, window_, gc_, x, y, width, height ); }
    void drawFilledRectangle( int x, int y, int width, int height );
    void drawString( int x, int y, const std::string &str );
    void copyArea( int src_x, int src_y, int width, int height,
      int dest_x, int dest_y );
    int textWidth( const std::string &str, int n )
        { return XTextWidth( font_, str.c_str(), n ); }
    int textWidth( const std::string &str )
        { return textWidth( str.c_str(), str.size() ); }
    int textAscent( void ) { return font_->ascent; }
    int textDescent( void ) { return font_->descent; }
    int textHeight( void ) { return textAscent() + textDescent(); }

    virtual void checkevent( void );
    void map( void ) { XMapWindow( display_, window_ ); }
    void unmap( void ) { XUnmapWindow( display_, window_ ); }
    void flush( void ) { XFlush( display_ ); }

    std::string getResource( const std::string &name ); // exit() if not found
    std::string getResourceOrUseDefault( const std::string &name,
      const std::string &defaultVal );
    bool isResourceTrue( const std::string &name );

    void dumpResources(std::ostream &os );

protected:
    class Event {
    public:
        Event( XWin *parent, int event, EventCallBack callBack );
        virtual ~Event( void ){}

        friend class XWin;

        void callBack( XEvent &event )
            { if ( event.type == event_ ) (parent_->*callBack_)( event ); }

    protected:
        XWin *parent_;
        EventCallBack callBack_;
        int event_;
        long mask_;
    private:
        Event *next_;
    };

    int x_, y_;                   //  position of the window
    int width_, height_;          //  width and height of the window
    Display       *display_;      //  Connection to X display
    Window        window_;        //  Application's main window
    GC            gc_;            //  The graphics context for the window
    XFontStruct   *font_;         //  Info on the default font
    std::string   name_;          //  Application's name
    XTextProperty title_;         //  Window name for title bar
    XTextProperty iconname_;      //  Icon name for icon label
    unsigned long fgcolor_;       //  Foreground color of the window
    unsigned long bgcolor_;       //  Background color of the window
    XWindowAttributes attr_;      //  Attributes of the window
    XSizeHints    *sizehints_;    //  Size hints for window manager
    Event         *events_;       //  List of Events for this window
    int           done_;          //  If true the application is finished.
    Atom          wm_, wmdelete_; //  Used to handle delete Events
    Colormap      colormap_;      //  The colormap
    std::string	display_name_;  //  Display name string.
    char*		geometry_;	//  geometry string.
    Xrm*		xrmptr_;	//  Pointer to the XOSView xrm.  FIXME???
    int		doStippling_;	//  Either 0 or 1.
    Pixmap	stipples_[4];	//  Array of Stipple masks.
    X11Graphics *_graphics;            //  New graphics interface

    void init( int argc, char *argv[] );
    void getGeometry( void );
    int getPixmap(Pixmap *);
    void setDisplayName (const std::string &new_display_name)
        { display_name_ = new_display_name; }
    const std::string &displayName() const { return display_name_; }

    void addEvent( Event *event );
    void setColors( void );
    void openDisplay( void );
    void setHints( int argc, char *argv[] );
    void setFont( void );
    void selectEvents( long mask );
    void ignoreEvents( long mask );
    void configureEvent( XEvent &event );
    void mappingNotify( XEvent &event )
        { XRefreshKeyboardMapping( &event.xmapping ); }
    void deleteEvent( XEvent &event );
    //void usage( void );
    Colormap colormap( void ) { return colormap_; }
    int screen( void ) { return DefaultScreen( display_ ); }
private:
};

inline void XWin::setStipple( Pixmap stipple) {
    if (!doStippling_)
        return;
    XSetStipple(display_, gc_, stipple);
    XGCValues xgcv;
    xgcv.fill_style = FillOpaqueStippled;
    XChangeGC (display_, gc_, GCFillStyle, &xgcv);
}

inline Pixmap XWin::createPixmap(const char* data, unsigned int w,
  unsigned int h) {
    return XCreatePixmapFromBitmapData(display_, window_,
      const_cast<char *>(data), w, h, 0, 1, 1);
}

inline void XWin::lineWidth( int width ) {
    XGCValues xgcv;
    xgcv.line_width = width;
    XChangeGC( display_, gc_, GCLineWidth, &xgcv );
}

inline void XWin::drawFilledRectangle( int x, int y, int width, int height ) {
    XFillRectangle( display_, window_, gc_, x, y, width + 1, height + 1 );
}

inline void XWin::drawString( int x, int y, const std::string &str ) {
    XDrawString( display_, window_, gc_, x, y, str.c_str(), str.size() );
}

inline void XWin::copyArea( int src_x, int src_y, int width, int height,
  int dest_x, int dest_y ) {
    XCopyArea( display_, window_, window_, gc_, src_x, src_y,
      width, height, dest_x, dest_y );
}

#endif
