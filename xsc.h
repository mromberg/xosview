//
//  Copyright (c) 2016
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef xsc_h
#define xsc_h

#include <string>

class XSCImp;


class XSessionClient {
public:
    XSessionClient(int argc, char * const *argv,
      const std::string &sessionArg="--smid");
    ~XSessionClient(void);

    // returns false if no session manager found.
    bool init(void);

    // returns true if die message received.
    bool check(int waitMsec=0, bool all=false);

    bool die(void) const;

    const std::string &sessionID(void) const;

private:
    XSCImp *_imp;

    // Not implemented.
    XSessionClient(const XSessionClient &xsc);
    XSessionClient &operator=(const XSessionClient &rhs);
};

#endif
