#ifndef LOG_H
#define LOG_H
//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

//
// iostream based loging for xosview.
//
// These macros log the file and line number along with the
// message provided in ostream format.
//
// Example useage:
//
// logEvent << "Good morning Vietnam!" << std::endl;
// logDebug << "2 + 2 = " << 2 + 2 << std::endl;
// logProblem << "I'm sorry, Dave. I'm afraid I can't do that." << std::endl;
// logBug << "I've just picked up a fault in the AE35 unit.\n"
//        << "It's going to go 100% failure in 72 hours." << std::endl;
// logFatal << "warp core breach imminent" << std::endl;
//
// Output:
//
//EVENT: main.cc:13: Good morning Vietnam!
//DEBUG: main.cc:14: 2 + 2 = 4
//PROBLEM: main.cc:15: I'm sorry, Dave. I'm afraid I can't do that.
//BUG: main.cc:16: I've just picked up a fault in the AE35 unit.
//It's going to go 100% failure in 72 hours.
//FATAL: main.cc:18: warp core breach imminent
//
// The configure script enables and defines XOSVDEBUG.  If not
// defined logDebug messages are disabled and the optimizer *should*
// remove the code from the executable entirely.
//
// logFatal will display it's message and then exit(1);
//
// The flags of the real underlying stream (std::cerr) are saved
//    and reset after each call.
//  So for example this works:
//
// logEvent << "hex    : " << std::hex << 42 << std::endl;
// logEvent << "not hex: " << 42 << std::endl;
// logEvent << "prec(3): " << std::setprecision(3) << 42.12345 << std::endl;
// logEvent << "normp  : " << 42.12345 << std::endl;
//
// Output:
//EVENT: main.cc:16: hex    : 2a
//EVENT: main.cc:17: not hex: 42
//EVENT: main.cc:18: prec(3): 42.1
//EVENT: main.cc:19: normp  : 42.1234
//
//  ASSERT  These are enabled by --enable-debug otherwise
// the compiler should optimize them out.
// logAssert(condition) << msg
// Example:
// logAssert(1 == 2) << "and dogs aren't cats." << std::endl;
// Output:
// ASSERT (1 == 2) false: main.cc:13: and dogs aren't cats.
// and the program exit()s
//
// Debug supressions:
//
//   When XOSDEBUG is defined all logDebug statements are active.
// This can be controlled by creating a simple config file in the working
// directory called log.conf.  The file is a sequence
// of lines that turn on or off logDebug messages from the
// source.
//
//  Each line is of the form "+|-  fspec".  The - lines suppress
// logging of files that match the fspec and lines that start with + turn
// logging back on.  The fspec is either a direct file name match or a
// glob match (if fglob is found on the system).  All lines are processed
// and each line overrides the preceeding ones.
//
//  For example this spec file disables all logDebugs except for those
// with "meter" in their name except for the netmeter:
// - *.cc
// + *meter*.cc
// - netmeter.cc
//
// At some point this could be extended to log to files (or elsewhere)
// and supress messages based on file/linenumber
//
#include <iostream>
#include <vector>
#include <utility>
#include <cstdlib>

#ifdef XOSVDEBUG
#define XOSVLOGIT true
#else
#define XOSVLOGIT false
#endif

namespace util {
// The operator <<= is used here because
// it's precedence is lower than <<.  So, the stream will
// be filled first.  We can then flush it and exit.
class Fatal {
public:
    void operator <<=(std::ostream &os) {
        os.flush();
        exit(1);
    }
};

// manipulator/object "thing" to save the state
// of a stream and restore it.  Inserted
// as first object into the real stream
// If C++ would ever add a "reset defaults" this
// kludge would not be necessary.
class ssave {
public:
    ssave(void) : _os(0), _state(0) {}
    ~ssave(void) { if (_os) _os->copyfmt(_state); }
    void sets(std::ostream &os) const {
        // operator<<(ostream, ssave) explodes if ssave is not const.
        // So... lie
        const_cast<ssave*>(this)->_os = &os;
        const_cast<ssave*>(this)->_state.copyfmt(os);
    }
private:
    std::ostream *_os;
    std::ios _state;   // May wanna make a global one
                       // to set the stream to default always
};

inline std::ostream &operator<<(std::ostream &os, const ssave &fs){
    fs.sets(os);
    return os;
}

class Log {
public:
    static bool suppress(const std::string &file, size_t lineNum);

private:
    static bool _readConfig;
    static std::vector<std::pair<std::string, bool> > _slist;

    static void readConfig(void);
};

} // end namespace util

//----------------------------------------------
// Basic macro for loging
// category: Name displayed at line start
// enabled : expression.  If true log the message
// ostr    : the ostream to log to.
//----------------------------------------------
#define logMsg(category,enabled,ostr) \
    if (enabled)                      \
        ostr << util::ssave()         \
             << category << ": "      \
             << __FILE__ << ":"       \
             << __LINE__ << ": "


// Events that are normal and are to be loged (always on)
#define logEvent logMsg("EVENT",true,std::cerr)

// Problems that can be worked around (always on)
#define logProblem logMsg("PROBLEM",true,std::cerr)

// Serious problems that can be worked around
// but may cause trouble elsewhere. (always on)
#define logBug logMsg("BUG",true,std::cerr)

// Serious problems that can not allow the
// program to continue in it's current state.
// After the message is loged, exit(1) is called  (always on)
#define logFatal logMsg("FATAL",true,util::Fatal()<<=std::cerr)

// Debug messages.
// Disabled unless XOSDEBUG defined.
// Then can be supressed via log.conf
#define logDebug logMsg("DEBUG",                        \
  (XOSVLOGIT&&!util::Log::suppress(__FILE__,__LINE__)), \
      std::cerr)

// Asserts
// Always on when XOSDEBUG is defined otherwise
// no ops (in production)
// condition : Expression that if false
//             sinks message to logFatal
// ------------------------------------
#define logAssert(condition)                                \
    if (XOSVLOGIT && (!(condition)))                        \
        util::Fatal()<<= std::cerr << "ASSERT ("            \
                     << #condition << ") false: "           \
                     << __FILE__ << ":" << __LINE__ << ": "

#endif
