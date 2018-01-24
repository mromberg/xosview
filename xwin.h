//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef xwin_h
#define xwin_h

#include <string>
#include <vector>
#include <memory>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

class X11Graphics;
class ResDB;



class XWin {
public:
    XWin(void);
    virtual ~XWin(void);

    int x(void) const { return _x; }
    void x(int val) { _x = val; }
    int y(void) const { return _y; }
    void y(int val) { _y = val; }
    unsigned int width(void) const { return _width; }
    void width(unsigned int val) { _width = val; }
    unsigned int height(void) const { return _height; }
    void height(unsigned int val) { _height = val; }

    void title(const std::string &str)
        { XStoreName(display(), window(), str.c_str()); }
    void iconname(const std::string &str)
        { XSetIconName(display(), window(), str.c_str()); }
    const std::string &appName(void) const { return _name; }

protected:
    X11Graphics &g(void) { return *_graphics; }
    const X11Graphics &g(void) const { return *_graphics; }

    Visual *visual(void) const { return _visual; }
    Display *display(void) const { return _display; }
    Window window(void) const { return _window; }
    virtual void setEvents(void);
    void createWindow(void);
    void setDisplayName(const std::string &name)
        { _displayName = name; }
    const std::string &displayName(void) const { return _displayName; }
    void setColors(void);
    void openDisplay(void);
    Colormap colormap(void) { return _colormap; }
    int screen(void) { return DefaultScreen(display()); }
    void appName(const std::string &name) { _name = name; }
    bool done(void) const { return _done; }
    void done(bool val) { _done = val; }
    void resize(int width, int height)
        { XResizeWindow(display(), window(), width, height); }
    void map(void) { XMapWindow(display(), window()); }
    void unmap(void) { XUnmapWindow(display(), window()); }
    void swapBB(void) const;

    // These return the configured color.  Not the current color.
    unsigned long foreground(void) { return _fgColor; }
    unsigned long background(void) { return _bgColor; }

    //------------------------------------------------------
    // Resouce interface
    //------------------------------------------------------
    virtual ResDB &resdb(void) = 0;
    //------------------------------------------------------

    //-----------------------------------
    //--- Events ------------------------
    //-----------------------------------
    typedef void (XWin::*EventCallBack)(XEvent &event);

    void addEvent(int eventType, XWin *xwin, EventCallBack callBack);
    void selectEvents(long mask);
    void ignoreEvents(long mask);
    void configureEvent(XEvent &event);
    void mappingNotify(XEvent &event)
        { XRefreshKeyboardMapping(&event.xmapping); }
    void deleteEvent(XEvent &event);
    virtual void checkevent(void);
    //-----------------------------------

private:
    class Event;  // defined below

    std::unique_ptr<X11Graphics> _graphics; //  New graphics interface
    bool               _done;           //  If true the application is finished.
    Atom               _wm, _wmdelete;  //  Used to handle delete Events
    std::string	       _displayName;    //  Display name string.
    int                _x, _y;          //  position of the window
    unsigned int       _width, _height; //  width and height of the window
    Visual            *_visual;         //  The X11 visual
    Display           *_display;        //  Connection to X display
    Window             _window;         //  Application's main window
    std::string        _name;           //  Application's name
    unsigned long      _fgColor;        //  Foreground color of the window
    unsigned long      _bgColor;        //  Background color of the window
    Colormap           _colormap;       //  The colormap
    std::vector<Event> _events;         //  List of Events for this window
    bool               _transparent;    //  If true has an ARGB visual
    bool               _dbe;            //  If true use double buffering
    Drawable           _bb;             //  The DBE back buffer


    class Event {
    public:
        Event(void) : _parent(0), _callBack(0), _event(0), _mask(NoEventMask)
            {}
        Event(XWin *parent, int event, EventCallBack callBack);

        // use compiler generated copy ctor and op

        void callBack(XEvent &event)
            { if (event.type == _event) (_parent->*_callBack)(event); }

        XWin *_parent;
        EventCallBack _callBack;
        int _event;
        long _mask;
    };


    Visual *getVisual(void);
    bool isDBE(Visual *v) const;
    std::vector<XVisualInfo> getVisuals(void);
    XSizeHints *getGeometry(void);
    void setHints(XSizeHints *szHints);
};

#endif
