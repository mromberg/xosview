//
//  Copyright (c) 1994, 1995, 2006, 2015 by Mike Romberg ( mike.romberg@noaa.gov )
//
//  This file may be distributed under terms of the GPL
//
#ifndef _Xrm_h
#define _Xrm_h

#include "bool.h"

#include <string>
#include <X11/Xlib.h>
#include <X11/Xresource.h>

class Xrm {
public:
  Xrm(const std::string &className, const std::string &instanceName);
  Xrm(const std::string &instanceName, int argc, char **argv);
  ~Xrm();

  std::string className(void) const
        { return std::string(XrmQuarkToString(_class)); }
  std::string instanceName(void) const
        { return std::string(XrmQuarkToString(_instance)); }

  const char *getResource(const std::string &rname) const;
  std::string getDisplayName (int argc, char** argv);
  void loadAndMergeResources(int& argc, char** argv, Display* display);

  std::ostream &dump(std::ostream &os) const;

private:
  XrmDatabase _db;
  XrmClass _class, _instance;
  std::string _display_name;  //  Used solely for getting the display's resources.

  //void getArgs(int argc, char **argv);
  static Bool enumCB(XrmDatabase *, XrmBindingList bindings,
                     XrmQuarkList quarks, XrmRepresentation *type,
                     XrmValue *value, XPointer closure);
  void initClassName(const std::string &className);

  static bool _initialized;
};

inline std::ostream &operator<<(std::ostream &os, const Xrm &xrm){
  return xrm.dump(os);
}

#endif
