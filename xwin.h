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
    const X11Graphics &g(void) const { return *_graphics; }
    X11Pixmap *newX11Pixmap(unsigned int width, unsigned int height);
    //-----------------------------------------------

    void resize( int width, int height )
        { XResizeWindow( display_, window_, width, height ); }




    //----------------------------------------
    // TODO
    //----------------------------------------

    //----------------------------------------

    virtual void checkevent( void );
    void map( void ) { XMapWindow( display_, window_ ); }
    void unmap( void ) { XUnmapWindow( display_, window_ ); }
    std::string getResource( const std::string &name ); // exit() if not found
    std::string getResourceOrUseDefault( const std::string &name,
      const std::string &defaultVal );
    bool isResourceTrue( const std::string &name );

    void dumpResources(std::ostream &os );

    //-------------------------------------------------
    // Depricated API
    //-------------------------------------------------
    void lineWidth( unsigned int width ) { g().lineWidth(width); }
    unsigned long allocColor( const std::string &name )
        { return g().allocColor(name); }
    unsigned long foreground( void ) { return g().fgPixel(); }
    unsigned long background( void ) { return g().bgPixel(); }
    void drawString( int x, int y, const std::string &str )
        { g().drawString(x, y, str); }
    void drawLine( int x1, int y1, int x2, int y2 )
        { g().drawLine(x1, y1, x2, y2); }
    void flush( void ) { g().flush(); }
    void clear( void ) { g().clear(); }
    void clear( int x, int y, unsigned int width, unsigned int height )
        { g().clear(x, y, width, height); }
    void drawRectangle(int x, int y, unsigned int width, unsigned int height)
        { g().drawRectangle(x, y, width, height); }
    void drawFilledRectangle( int x, int y,
      unsigned int width, unsigned int height )
        { g().drawFilledRectangle(x, y, width, height); }
    void copyArea( int src_x, int src_y,
      unsigned int width, unsigned int height, int dest_x, int dest_y )
        { g().copyArea(src_x, src_y, width, height, dest_x, dest_y); }
    void setStipple( Pixmap stipple) { g().setStipple(stipple); }
    void setStippleN (int n) { g().setStippleN(n); }
    Pixmap createPixmap(const std::string &data,
      unsigned int w, unsigned int h) { return g().createPixmap(data, w, h); }
    void setForeground( unsigned long pixelvalue )
        { g().setForeground(pixelvalue); }
    void setBackground( unsigned long pixelvalue )
        { g().setBackground(pixelvalue); }
    unsigned int textWidth( const std::string &str, int n )
        { return g().textWidth(str.substr(0, n)); }
    unsigned int textWidth( const std::string &str )
        { return g().textWidth(str); }
    int textAscent( void ) const { return g().textAscent(); }
    int textDescent( void ) const { return g().textDescent(); }
    unsigned int textHeight( void ) const { return g().textHeight(); }
    //-End Depricated----------------------------------

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
    std::string   name_;          //  Application's name
    XTextProperty title_;         //  Window name for title bar
    XTextProperty iconname_;      //  Icon name for icon label
private:
    unsigned long fgcolor_;       //  Foreground color of the window
    unsigned long bgcolor_;       //  Background color of the window
    Colormap      colormap_;      //  The colormap
protected:
    XWindowAttributes attr_;      //  Attributes of the window
    XSizeHints    *sizehints_;    //  Size hints for window manager
    Event         *events_;       //  List of Events for this window
    int           done_;          //  If true the application is finished.
    Atom          wm_, wmdelete_; //  Used to handle delete Events

    std::string	display_name_;  //  Display name string.
    char*		geometry_;	//  geometry string.
    Xrm*		xrmptr_;	//  Pointer to the XOSView xrm.  FIXME??


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
    X11Graphics *_graphics;            //  New graphics interface
};

#endif
