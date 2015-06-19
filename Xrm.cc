//
//  Copyright (c) 1994, 1995, 2006, 2008, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include <stdlib.h>
#include <ctype.h>
#ifdef HAVE_IOSTREAM
#include <iostream>
#else
#include <iostream.h>
#endif
#include <unistd.h>  //  for access(), etc.  BCG
#include "general.h"
#include <cstddef> // for NULL
#include "Xrm.h"
#include "Xrmcommandline.h"
#include "strutil.h"
#include "log.h"

extern char *defaultXResourceString;

bool Xrm::_initialized = false;

Xrm::Xrm(const std::string &instanceName, int argc, char **argv){
    logFatal << " Error:  This constructor is not supported yet." << std::endl;
    _db = NULL;
    _class = _instance = NULLQUARK;
    getDisplayName(argc, argv);

    (void) instanceName;
    //  End unsupported constructor.  !!!!!!!! BCG
}

Xrm::Xrm(const std::string &className, const std::string &instanceName){
    XrmInitialize ();

    //  Initialize everything to NULL.
    _db = NULL;
    _class = _instance = NULLQUARK;

    // init the _instance and _class Quarks
    _instance = XrmStringToQuark(instanceName.c_str());
    initClassName(className);
}


std::string Xrm::getDisplayName (int argc, char** argv) {
    (void) argc;  //  Avoid gcc warnings.
    //  See if '-display foo:0' is on the command line, and return it if it is.
    char** argp;

    for (argp = argv; (*argp != NULL) &&
             (util::tolower(*argp) != "-display") ; argp++)
        ;  //  Don't do anything.

    //  If we found -display and the next word exists...
    if (*argp && *(++argp))
        _display_name = *argp;
    else
        _display_name = "";
    return _display_name.c_str();
    //  An empty display string means use the DISPLAY environment variable.
}

Xrm::opt Xrm::getResource(const std::string &rname) const{
    std::string frn = std::string(instanceName()) + std::string(".") + rname;
    std::string fcn = std::string(className()) + std::string(".") + rname;

    XrmValue val;
    val.addr = NULL;
    char *type;
    XrmGetResource(_db, frn.c_str(), fcn.c_str(), &type, &val);
    //  This case here is a hack, because we are currently moving from
    //  always making the instance name be "xosview" to allowing
    //  user-specified ones.  And unfortunately, the class name is
    //  XOsview, and not xosview, so our old defaults (xosview.font)
    //  will not be found when searching for XOsview.font.  bgrayson Dec. 1996
    if (!val.addr) {
        //  Let's try with a non-uppercased class name.
        std::string fcn_lower(className());
        for (unsigned int i = 0 ; i < fcn_lower.size() ; i++)
            fcn_lower[i] = std::tolower(fcn_lower[i]);
        fcn = fcn_lower + std::string(".") + rname;
        XrmGetResource(_db, frn.c_str(), fcn.c_str(), &type, &val);
    }

    if (val.addr)
        return opt(true, util::strip(val.addr));
    return opt(false, "<(Xrm::uninitialized)>");
}

Xrm::~Xrm(){
    XrmDestroyDatabase(_db);
}

//---------------------------------------------------------------------
//  This function uses XrmParseCommand, and updates argc and argv through it.
void Xrm::loadAndMergeResources(int& argc, char** argv, Display* display){

    // init the database if it needs it
    if (!_initialized){
        XrmInitialize();
        _initialized = true;
    }
    else {
        logFatal << "Xrm:loadAndMergeResources() called twice!"
                 << std::endl;
    }
    //  This is ugly code.  According to X and Xt rules, many files need
    //  to be checked for resource settings.  Since we aren't (yet) using
    //  Xt or any other package, we need to do all of these checks
    //  individually.  BCG
//  =========== BEGIN X Resource lookup and merging ==========

//  This all needs to be done in the proper order:
/*
  Listed from weakest to strongest:
  (from code-builtin-resources) (Stored in the string in defaultstring.h)
  app-defaults
  XOSView (from XAPPLRESDIR directory)
  from RESOURCE_MANAGER property on server (reads .Xdefaults if needed)
  from file specified in XENVIRONMENT
  from command line (i.e., handled with XrmParseCommand)
*/

    // Get resources from the various resource files

    //  Put the default, compile-time options as the lowest priority.
    _db = XrmGetStringDatabase (defaultXResourceString);

    //  Merge in the system resource database.
    std::string rfilename;

    // Get the app-defaults
    if (XrmQuarkToString(_class)) {
        rfilename = std::string("/etc/X11/app-defaults/")
            + XrmQuarkToString(_class);
        XrmCombineFileDatabase(rfilename.c_str(), &_db, 1);

        rfilename = std::string("/usr/lib/X11/app-defaults/")
            + XrmQuarkToString(_class);
        XrmCombineFileDatabase (rfilename.c_str(), &_db, 1);

        rfilename = std::string("/usr/X11R6/lib/X11/app-defaults/")
            + XrmQuarkToString(_class);
        XrmCombineFileDatabase (rfilename.c_str(), &_db, 1);

        //  Try a few more, for SunOS/Solaris folks.
        rfilename = std::string("/usr/openwin/lib/X11/app-defaults/")
            + XrmQuarkToString(_class);
        XrmCombineFileDatabase (rfilename.c_str(), &_db, 1);

        rfilename = std::string("/usr/local/X11R6/lib/X11/app-defaults/")
            + XrmQuarkToString(_class);
        XrmCombineFileDatabase (rfilename.c_str(), &_db, 1);
    }



    //  Now, check for an XOSView file in the XAPPLRESDIR directory...
    char* xappdir = getenv ("XAPPLRESDIR");
    if (xappdir != NULL) {
        std::string xappfile;
        xappfile = std::string(xappdir) + "/" + className();
        // this did not work for XAPPLRESDIR
        //if (!access (xappfile, X_OK | R_OK))
        if (!access(xappfile.c_str(), R_OK)) {
            XrmCombineFileDatabase(xappfile.c_str(), &_db, 1);
        }
    }

    //  Now, check the display's RESOURCE_MANAGER property...
    char* displayString = XResourceManagerString (display);
    if (displayString != NULL) {
        XrmDatabase displayrdb = XrmGetStringDatabase (displayString);
        XrmMergeDatabases (displayrdb, &_db);  //Destroys displayrdb when done.
    }

    //  And check this screen of the display...
    char* screenString =
        XScreenResourceString (DefaultScreenOfDisplay(display));
    if (screenString != NULL) {
        XrmDatabase screenrdb = XrmGetStringDatabase (screenString);
        XrmMergeDatabases (screenrdb, &_db);  //  Destroys screenrdb when done.
    }

    //  Now, check for a user resource file, and merge it in if there is one...
    char *home = getenv("HOME");
    if (home){
        std::string userrfilename = std::string(home) + "/.Xdefaults";

        //  User file overrides system (_db).
        XrmCombineFileDatabase (userrfilename.c_str(), &_db, 1);
    }

    //  Second-to-last, parse any resource file specified in the
    //  environment variable XENVIRONMENT.
    char* xenvfile;
    if ((xenvfile = getenv ("XENVIRONMENT")) != NULL) {
        //  The XENVIRONMENT file overrides all of the above.
        XrmCombineFileDatabase (xenvfile, &_db, 1);
    }
    //  Command-line resources override system and user defaults.
    XrmDatabase cmdlineRdb_ = NULL;
    XrmParseCommand (&cmdlineRdb_, options, NUM_OPTIONS, instanceName().c_str(),
		    &argc, argv);
    XrmCombineDatabase (cmdlineRdb_, &_db, 1);  //  Keeps cmdlineRdb_ around.
//  =========== END X Resource lookup and merging ==========
}

void Xrm::initClassName(const std::string &name){
    std::string className(name);
    className[0] = std::toupper(className[0]);

    if (className[0] == 'X')
        className[1] = std::toupper(className[1]);

    _class = XrmStringToQuark(className.c_str());
}



//------------  Some debugging functions follow.  -----------------------
inline std::ostream &operator<<(std::ostream &os, const XrmBinding &b){
    switch (b){
    case XrmBindTightly:
        return os << ".";
    case XrmBindLoosely:
        return os << "*";
    default:
        logBug << "std::ostream operator<<(std::ostream &, "
               << "const XrmBinding &) : "
               << "Unknown XrmBinding!";
        return os;
    }

    return os;
}

std::ostream &Xrm::dump(std::ostream &os) const {
    os <<"--- Xrm --- class: " <<XrmQuarkToString(_class)
       <<", instance: " <<XrmQuarkToString(_instance) <<"\n";

    XrmName names[] = { _instance, NULLQUARK };
    XrmClass classes[] = { _class, NULLQUARK };

    XrmEnumerateDatabase(_db, names, classes, XrmEnumAllLevels, enumCB,
      (XPointer)&os);

    return os;
}

Bool Xrm::enumCB(XrmDatabase *, XrmBindingList bindings,
  XrmQuarkList quarks, XrmRepresentation *type,
  XrmValue *value, XPointer closure) {

    std::ostream *os = (std::ostream *)closure;
    (void) type;  //  Avoid gcc warnings.

    int i = 0;
    while (quarks[i] != NULLQUARK){
        *os <<bindings[i] <<XrmQuarkToString(quarks[i]);
        i++;
    }
    *os <<": " <<value->addr <<"\n";

    return False;
}
