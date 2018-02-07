//
//  Copyright (c) 1994, 1995, 2006, 2008, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "Xrm.h"
#include "strutil.h"
#include "log.h"

#include <unistd.h> // getenv()

extern char *defaultXResourceString;  // from defaultstring.cc



// This *should* be set by configure through the Makefile
#ifdef XAPPLOADDIR
static const char *XAppLoadDir = XAPPLOADDIR;
#else
static const char *XAppLoadDir = nullptr;
#endif


Xrm::Xrm(const std::string &className, const std::string &instanceName)
    : _db(0), _class(NULLQUARK), _instance(NULLQUARK) {

    initialize(className, instanceName);
}


ResDB::opt Xrm::getOptResource(const std::string &rname) const {

    std::string frn = instanceName() + "." + rname;
    std::string fcn = className() + "." + rname;

    XrmValue val;
    val.addr = nullptr;
    char *type;
    XrmGetResource(_db, frn.c_str(), fcn.c_str(), &type, &val);
    //  This case here is a hack, because we are currently moving from
    //  always making the instance name be "xosview" to allowing
    //  user-specified ones.  And unfortunately, the class name is
    //  XOsview, and not xosview, so our old defaults (xosview.font)
    //  will not be found when searching for XOsview.font.  bgrayson Dec. 1996
    if (!val.addr) {
        //  Let's try with a non-uppercased class name.
        fcn = util::tolower(className()) + '.' + rname;
        XrmGetResource(_db, frn.c_str(), fcn.c_str(), &type, &val);
    }

    if (val.addr) {
        return opt(true, fixValue(val.addr));
    }

    return opt(false, "<(Xrm::uninitialized)>");
}


std::string Xrm::fixValue(const std::string &val) {
    // Support '!' as a comment in the resorce value.
    // Allow it to be used by escapeing it with '&'
    // (ex: "True ! This is a boolean" = "True"
    // "OMG&! A Pony&! ! awesome!" = "OMG! A Pony!"
    //
    // '&' was chosen 'cause xrdb seems to use '\' for something
    // Also strips whitespace from front and back.
    std::string rval;

    for (const auto &v : util::split(val, "!")) {
        if (!v.empty() && (v.back() == '&')) {
            // Then ! was escaped
            rval += util::rstrip(v, "&") + "!";
        }
        else {
            // was a comment.
            rval += v;
            break;
        }
    }

    return util::strip(rval);
}


Xrm::~Xrm(void) {
    logDebug << "Xrm::~Xrm(): " << (void *)_db << std::endl;
    if (_db)
        XrmDestroyDatabase(_db);
}


void Xrm::loadResources(Display* display) {
    // Load resources with each subsequent load overriding the last.
    // This more or less follows the order in the X11 documentation.
    //
    // Listed from weakest to strongest:
    //  *(from code-builtin-resources) (Stored in the string in defaultstring.h)
    //  * app-defaults directory
    //  * XOSView (from XAPPLRESDIR directory)
    //  * from RESOURCE_MANAGER property on server (reads .Xdefaults if needed)
    //  * from file specified in XENVIRONMENT


    // Get resources from the various resource files

    //  Put the default, compile-time options as the lowest priority.
    _db = XrmGetStringDatabase(defaultXResourceString);

    //------------------------------------------
    //  Merge in the system resource database.
    //------------------------------------------

    // Get the app-defaults
    // This one is set by configure
    // We will look in this spot (and this spot ONLY
    // If for some reason it is not set, complain and move on
    if (XAppLoadDir) {
        loadResources(std::string(XAppLoadDir) + "/" + className());
    }
    else {
        logProblem << "XAPPLRESDIR was not correctly configured/set "
                   << "at build time.  Skipping app-defaults..."
                   << std::endl;
    }

    //  Now, check for an XOSView file in the XAPPLRESDIR directory...
    loadFromEnv("XAPPLRESDIR", "/" + className());

    //  Now, check the display's RESOURCE_MANAGER property...
    const char *displayString = XResourceManagerString(display);
    if (displayString != nullptr) {
        XrmDatabase displayrdb = XrmGetStringDatabase(displayString);
        XrmMergeDatabases(displayrdb, &_db);  //Destroys displayrdb when done.
        logDebug << "loaded display resources." << std::endl;
    }

    //  And check this screen of the display...
    char *screenString =
        XScreenResourceString(DefaultScreenOfDisplay(display));
    if (screenString != nullptr) {
        XrmDatabase screenrdb = XrmGetStringDatabase(screenString);
        XrmMergeDatabases(screenrdb, &_db);  //  Destroys screenrdb when done.
        XFree(screenString); // unlike XResourceManagerString() we must free
        logDebug << "loaded screen resources." << std::endl;
    }

    //  Now, check for a user resource file, and merge it in if there is one...
    loadFromEnv("HOME", "/.Xdefaults");

    //  Second-to-last, parse any resource file specified in the
    //  environment variable XENVIRONMENT.
    loadFromEnv("XENVIRONMENT");
}


void Xrm::initQuarks(const std::string &cName,
  const std::string &instanceName) {

    logAssert(cName.size() >= 2) << "Invalid class name." << std::endl;

    std::string className(cName);
    className[0] = std::toupper(className[0]);

    if (className[0] == 'X')
        className[1] = std::toupper(className[1]);

    _class = XrmStringToQuark(className.c_str());
    if (_class == NULLQUARK)
        logFatal << "NULL class quark: " << className << std::endl;
    _instance = XrmStringToQuark(instanceName.c_str());
    if (_instance == NULLQUARK)
        logFatal << "NULL instance quark: " << instanceName << std::endl;

    // From the man page:
    // "For any given quark, if XrmStringToQuark returns a non-NULL value, all
    // future calls will return the same value (identical address)"
    //
    // We will check them once here...
    if (!XrmQuarkToString(_class))
        logFatal << "failed to convert class quark." << std::endl;
    if (!XrmQuarkToString(_instance))
        logFatal << "failed to convert instance quark." << std::endl;
}



//------------  Some debugging functions follow.  -----------------------
inline std::ostream &operator<<(std::ostream &os, const XrmBinding &b) {
    switch (b) {
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

    std::array<XrmName, 2> names = { _instance, NULLQUARK };
    std::array<XrmClass, 2> classes = { _class, NULLQUARK };

    std::vector<std::string> rlist;
    XrmEnumerateDatabase(_db, names.data(), classes.data(), XrmEnumAllLevels,
      enumCB, reinterpret_cast<XPointer>(&rlist));

    std::sort(rlist.begin(), rlist.end());

    for (const auto &r : rlist)
        os << r << "\n";

    return os;
}


Bool Xrm::enumCB(XrmDatabase *, XrmBindingList bindings,
  XrmQuarkList quarks, XrmRepresentation *type,
  XrmValue *value, XPointer closure) {

    (void) type;  //  Avoid warnings.

    std::vector<std::string> *rlist =
        reinterpret_cast<std::vector<std::string> *>(closure);

    std::string res;
    size_t i = 0;
    while (quarks[i] != NULLQUARK) {
        res += util::repr(bindings[i])
            + std::string(XrmQuarkToString(quarks[i]));
        i++;
    }

    res += ":\t" + Xrm::fixValue(value->addr);
    rlist->push_back(res);

    return False;
}


void Xrm::putResource(const std::string &line) {
    XrmPutLineResource(&_db, line.c_str());
}


void Xrm::putResource(const std::string &specifier, const std::string &val) {
    XrmPutStringResource(&_db, specifier.c_str(), val.c_str());
}


bool Xrm::loadResources(const std::string &fname) {
    bool rval = XrmCombineFileDatabase(fname.c_str(), &_db, 1);
    logDebug << "combine: " << fname << " : " << rval << std::endl;
    return rval;
}


void Xrm::initialize(const std::string &className,
  const std::string &instanceName) {

    static bool first = true;
    if (first) {
        first = false;
        XrmInitialize();
    }

    // init the _instance and _class Quarks
    initQuarks(className, instanceName);
}


void Xrm::loadFromEnv(const std::string &vname, const std::string &post) {
    const char *eval = getenv(vname.c_str());
    if (eval != nullptr)
        loadResources(eval + post);
}
