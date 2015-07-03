//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef XRM_H
#define XRM_H

#include <string>
#include <utility>
#include <X11/Xlib.h>
#include <X11/Xresource.h>

class Xrm {
public:
    Xrm(const std::string &className, const std::string &instanceName);
    ~Xrm();

    std::string className(void) const
        { return std::string(XrmQuarkToString(_class)); }
    std::string instanceName(void) const
        { return std::string(XrmQuarkToString(_instance)); }

    // At some point ISO C++ may adopt std::optional.  At which point
    // optional would be slightly clearner.  For now, this works ok
    // to indicate non-existant resources.
    typedef std::pair<bool, std::string> opt;

    opt getResource(const std::string &rname) const;
    void putResource(const std::string &line);
    void putResource(const std::string &specifier, const std::string &val);
    void loadResources(Display* display);

    std::ostream &dump(std::ostream &os) const;

private:
    XrmDatabase _db;
    XrmClass _class, _instance;

    //void getArgs(int argc, char **argv);
    static Bool enumCB(XrmDatabase *, XrmBindingList bindings,
      XrmQuarkList quarks, XrmRepresentation *type,
      XrmValue *value, XPointer closure);
    void initClassName(const std::string &className);
    static std::string fixValue(const std::string &val);

    static bool _initialized;
};

inline std::ostream &operator<<(std::ostream &os, const Xrm &xrm){
    return xrm.dump(os);
}

#endif
