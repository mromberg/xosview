//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef _Host_h
#define _Host_h

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#ifdef HAVE_IOSTREAM
#include <iostream>
#else
#include <iostream.h>
#endif
#include "bool.h"

class Host {
public:
    Host(const std::string &hostname);
    Host(const struct in_addr *address);
    Host(unsigned int addr);
    Host(const Host& host) { copy(host); _failure = host._failure; }
    virtual ~Host(void);

    Host &operator=(const Host& host);

    bool operator==(const Host& host) const;
    bool operator!=(const Host& host) const { return !(*this == host); }

    // indicates a valid Host.
    bool valid(void) const { return _valid; }
    operator bool(void) const { return _valid; }
    int reasonForFailure(void) const;  // returns h_errno at time of failure
    bool tryAgain(void) const;         // Ok to try again?

    std::string officialName(void) const { return std::string(_hent.h_name); }

    int numAliases(void) const { return _numAliases; }
    std::string alias(int num) const
        { return std::string(_hent.h_aliases[num]); }

    int addrType(void) const { return _hent.h_addrtype; }
    int addrLength(void) const { return _hent.h_length; }

    int numAddresses(void) const { return _numAddresses; }
    struct in_addr *address(int num) const
        { return (in_addr *)_hent.h_addr_list[num]; }

    // Linux will choke and die inside of inet_ntoa() when
    // this function is called.  The Host class has been run
    // through purify and the problem is not in it.  Must be
    // another linux library problem :(.
    std::string strAddress(int num) const
        { return std::string(inet_ntoa(*address(num))); }

    // Should not use this under linux for the same reashon as the above
    // function.
    std::ostream &print(std::ostream &os) const;

protected:
private:
    struct hostent _hent;
    int _numAliases;
    int _numAddresses;
    bool _valid;
    int  _failure;

    void copy(const Host& host) { copy(&host._hent); }
    void copy(const struct hostent *hent);
    void clear(void);
    bool check(const struct hostent *hent);
    bool constuct(const struct in_addr *address);
};

// Do not use this under linux until inet_ntoa() is fixed.
inline std::ostream &operator<<(std::ostream &os, const Host& host) {
    return host.print(os);
}

inline Host &Host::operator=(const Host& host) {
    clear();
    copy(host);
    _failure = host._failure;

    return *this;
}

#endif
