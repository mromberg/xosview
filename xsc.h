//
//  Copyright (c) 2016, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef xsc_h
#define xsc_h

#include <string>
#include <vector>
#include <memory>

class XSCImp;


class XSessionClient {
public:
    XSessionClient(const std::vector<std::string> &argv,
      const std::string &sessionArg, const std::string &lastID);
    XSessionClient(const std::vector<std::string> &argv,
      const std::string &sessionArg="--smid");
    ~XSessionClient(void);

    XSessionClient(const XSessionClient &xsc) = delete;
    XSessionClient &operator=(const XSessionClient &rhs) = delete;

    // returns false if no session manager found.
    bool init(void);

    // returns true if die message received.
    bool check(int waitMsec=0, bool all=false);

    bool die(void) const;

    const std::string &sessionID(void) const;

private:
    std::unique_ptr<XSCImp> _imp;
};

#endif
