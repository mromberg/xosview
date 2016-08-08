//
//  Copyright (c) 1994, 1995, 2015, 2016
//  by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  NetBSD port:
//  Copyright (c) 1995, 1996, 1997-2002 by Brian Grayson (bgrayson@netbsd.org)
//
//  This file was written by Brian Grayson for the NetBSD and xosview
//    projects.
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//
#ifndef NETMETER_H
#define NETMETER_H

#include "cnetmeter.h"


class NetMeter : public ComNetMeter {
public:
    NetMeter( void );

    void checkResources(const ResDB &rdb);

protected:
    virtual std::pair<float, float> getRates(void);

private:
    uint64_t _lastBytesIn, _lastBytesOut;
    std::string _netIface;
    bool _ignored;

    bool ifskip(const std::string &ifname, const std::string &netIface,
      bool ignored) const;
    void getNetInOut(uint64_t &inbytes, uint64_t &outbytes,
      const std::string &netIface, bool ignored) const;
};


inline bool NetMeter::ifskip(const std::string &ifname,
  const std::string &netIface, bool ignored) const {

    return (netIface != "False") &&
        ((!ignored && netIface != ifname) || (ignored && netIface == ifname));
}


#endif
