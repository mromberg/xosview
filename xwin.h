#ifndef XWIN_H
#define XWIN_H
#include "configxosv.h"
#include "x11graphics.h"

#include <string>

#include <X11/Xlib.h>
#include <X11/Xutil.h>



class XWin;
class X11Pixmap;

typedef void (XWin::*EventCallBack)( XEvent &event );


class XWin {
public:
    XWin(void);
    virtual ~XWin(void);

    int x(void) const { return x_; }
    void x(int val) { x_ = val; }
    int y(void) const { return y_; }
    void y(int val) { y_ = val; }
    unsigned int width(void) const { return width_; }
    void width(unsigned int val) { width_ = val; }
    unsigned int height(void) const { return height_; }
    void height(unsigned int val) { height_ = val; }

    // These return the configured color.  Not the current color.
    unsigned long foreground(void) { return fgcolor_; }
    unsigned long background(void) { return bgcolor_; }

    Display *display(void) { return display_; }
    Window window(void) { return window_; }

    //------------------------------------------------------
    // Resouce interface
    //------------------------------------------------------
    virtual std::string getResource(const std::string &name); // exit not found
    virtual std::string getResourceOrUseDefault(const std::string &name,
      const std::string &defaultVal);
    virtual bool isResourceTrue(const std::string &name);
    virtual void dumpResources(std::ostream &os);
    //------------------------------------------------------

    void title(const std::string &str)
        { XStoreName( display_, window_, str.c_str() ); }
    void iconname(const std::string &str)
        { XSetIconName( display_, window_, str.c_str() ); }
    const std::string &appName(void) const { return name_; }

    //-----------------------------------------------
    // New Graphics interface
    // Drawing should be done by using a member of g()
    // Preferable rewrite the meters to pass an
    // reference/pointer to g() to their draw methods.
    //-----------------------------------------------
    X11Graphics &g(void) { return *_graphics; }
    const X11Graphics &g(void) const { return *_graphics; }
    X11Pixmap *newX11Pixmap(unsigned int width, unsigned int height);
    //-----------------------------------------------

protected:
    virtual void setEvents(void);
    void createWindow(void);

    virtual std::string className(void);
    virtual std::string instanceName(void);

    bool getPixmap(Pixmap *, const std::string &pixmapFName);
    void setDisplayName(const std::string &new_display_name)
        { display_name_ = new_display_name; }
    const std::string &displayName(void) const { return display_name_; }
    void setColors(void);
    void openDisplay(void);
    Colormap colormap(void) { return colormap_; }
    int screen(void) { return DefaultScreen( display_ ); }
    void appName(const std::string &name) { name_ = name; }
    bool done(void) const { return done_; }
    void done(bool val) { done_ = val; }
    void resize( int width, int height )
        { XResizeWindow( display_, window_, width, height ); }
    void map( void ) { XMapWindow( display_, window_ ); }
    void unmap( void ) { XUnmapWindow( display_, window_ ); }

    //-----------------------------------
    //--- Events ------------------------
    //-----------------------------------
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
    void addEvent(Event *event);
    void selectEvents(long mask);
    void ignoreEvents(long mask);
    void configureEvent(XEvent &event);
    void mappingNotify(XEvent &event)
        { XRefreshKeyboardMapping( &event.xmapping ); }
    void deleteEvent(XEvent &event);
    virtual void checkevent( void );
    //-----------------------------------

private:
    X11Graphics *_graphics;       //  New graphics interface
    Event         *events_;       //  List of Events for this window
    bool           done_;         //  If true the application is finished.
    Atom          wm_, wmdelete_; //  Used to handle delete Events
    std::string	display_name_;    //  Display name string.
    int x_, y_;                   //  position of the window
    unsigned int width_, height_; //  width and height of the window
    Display       *display_;      //  Connection to X display
    Window        window_;        //  Application's main window
    std::string   name_;          //  Application's name
    unsigned long fgcolor_;       //  Foreground color of the window
    unsigned long bgcolor_;       //  Background color of the window
    Colormap      colormap_;      //  The colormap

    XSizeHints *getGeometry(void);
    void setHints(XSizeHints *szHints);
};

#endif
