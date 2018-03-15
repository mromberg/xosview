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
#include <functional>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

class X11Graphics;
class ResDB;



class XWin {
public:
    XWin(void);
    virtual ~XWin(void);

    unsigned int width(void) const { return _width; }
    unsigned int height(void) const { return _height; }
    const std::string &appName(void) const { return _name; }

protected:
    X11Graphics &g(void) { return *_graphics; }
    const X11Graphics &g(void) const { return *_graphics; }

    void width(unsigned int val) { _width = val; }
    void height(unsigned int val) { _height = val; }
    Visual *visual(void) const { return _visual; }
    Display *display(void) const { return _display; }
    int screen(void) { return DefaultScreen(display()); }
    Window window(void) const { return _window; }
    Colormap colormap(void) { return _colormap; }

    void appName(const std::string &name) { _name = name; }
    void title(const std::string &str)
        { XStoreName(display(), window(), str.c_str()); }
    void iconname(const std::string &str)
        { XSetIconName(display(), window(), str.c_str()); }

    void createWindow(void);
    void setDisplayName(const std::string &name)
        { _displayName = name; }
    const std::string &displayName(void) const { return _displayName; }
    void setColors(void);
    void openDisplay(void);

    bool done(void) const { return _done; }
    void done(bool val) { _done = val; }
    void resize(int width, int height);
    void map(void);
    void unmap(void);
    void swapBB(void) const;

    // These return the configured color.  Not the current color.
    unsigned long foreground(void) { return _fgColor; }
    unsigned long background(void) { return _bgColor; }

    // Resouce interface
    virtual ResDB &resdb(void) = 0;

    //-----------------------------------
    //--- Events ------------------------
    //-----------------------------------
    using Callback = std::function<void(const XEvent &)>;

    virtual void setEvents(void);

    void addEvent(int eventType, const Callback &callback);
    void selectEvents(long mask);
    void ignoreEvents(long mask);
    void configureEvent(const XEvent &event);
    void mappingNotify(const XEvent &event);
    void deleteEvent(const XEvent &event);
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
        Event(int event, const Callback &cb) : _cb(cb), _event(event) {}

        // use compiler generated copy ctor and op

        void callBack(const XEvent &event)
            { if (event.type == _event) _cb(event); }

        Callback _cb;
        int _event;

        long mask(void) const;
    };


    Visual *getVisual(void);
    bool isDBE(Visual *v) const;
    XSizeHints *getGeometry(void);
    void setHints(XSizeHints *szHints);
};

#endif
