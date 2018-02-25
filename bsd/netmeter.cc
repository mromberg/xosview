//
//  Copyright (c) 1994, 1995, 2015, 2016, 2018
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
#include "netmeter.h"

#if defined(XOSVIEW_FREEBSD) || defined(XOSVIEW_DFBSD)
#include "sctl.h"

#include <net/if.h>
#include <net/if_mib.h>
#endif

#if defined(XOSVIEW_NETBSD)
#include <cstring>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <machine/int_fmtio.h>
#endif

#if defined(XOSVIEW_OPENBSD)
#include <sys/sysctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/route.h>
#include <net/if_dl.h>
#endif


NetMeter::NetMeter( void )
    : ComNetMeter(),
      _lastBytesIn(0), _lastBytesOut(0),
      _netIface("False"), _ignored(false) {
#if defined(XOSVIEW_NETBSD)
    if ((_socket = socket(AF_LOCAL, SOCK_DGRAM, 0)) == -1)
        logFatal << "socket(AF_LOCAL, SOCK_DGRAM, 0) failed: "
                 << util::strerror() << std::endl;
#endif
}


NetMeter::~NetMeter(void) {
#if defined(XOSVIEW_NETBSD)
    if (_socket != -1)
        close(_socket);
#endif
}


void NetMeter::checkResources(const ResDB &rdb) {

    ComNetMeter::checkResources(rdb);

    _netIface = rdb.getResource("netIface");
    if (_netIface[0] == '-') {
        _ignored = true;
        _netIface.erase(0, _netIface.find_first_not_of("- "));
    }
}


std::pair<float, float> NetMeter::getRates(void) {
    static bool first = true;
    if (first) {
        first = false;
        getNetInOut(_lastBytesIn, _lastBytesOut, _netIface, _ignored);
        timerStart();
        return std::make_pair(0.0, 0.0);
    }

    uint64_t nowBytesIn, nowBytesOut;

    timerStop();
    getNetInOut(nowBytesIn, nowBytesOut, _netIface, _ignored);
    double t = 1.0 / etimeSecs();
    timerStart();

    std::pair<float, float> rval((nowBytesIn - _lastBytesIn) * t,
      (nowBytesOut - _lastBytesOut) * t);
    _lastBytesIn = nowBytesIn;
    _lastBytesOut = nowBytesOut;

    return rval;
}


#if defined(XOSVIEW_NETBSD)
// -----------------------------------------------------------
// Based on a code snippet from Martin Husemann on the NetBSD
// forums.
// -----------------------------------------------------------
void NetMeter::getNetInOut(uint64_t &inbytes, uint64_t &outbytes,
  const std::string &netIface, bool ignored) const {

    inbytes = outbytes = 0;
    struct if_nameindex *iflist = if_nameindex();
//    int s = socket(AF_LOCAL, SOCK_DGRAM, 0);

    for (const struct if_nameindex *p = iflist; p->if_index > 0; p++) {
        struct ifdatareq ifdr;
        memset(&ifdr, 0, sizeof(ifdr));
        std::string p_if_name(p->if_name);
        memcpy(ifdr.ifdr_name, p_if_name.c_str(), p_if_name.length());
        if (ioctl(_socket, SIOCGIFDATA, &ifdr) == -1) {
            logFatal << "ioctl(SIOCGIFDATA) failed for: " << p->if_name
                     << std::endl;
        }
        std::string ifname(ifdr.ifdr_name);

        if (ifskip(ifname, netIface, ignored))
            continue;

        const struct if_data *ifi = &ifdr.ifdr_data;
        inbytes += ifi->ifi_ibytes;
        outbytes += ifi->ifi_obytes;

        logDebug << ifname
                 << " in: " << ifi->ifi_ibytes
                 << " out: " << ifi->ifi_obytes << std::endl;
    }

//    close(s);
    if_freenameindex(iflist);
}
#endif


#if defined(XOSVIEW_OPENBSD)
void NetMeter::getNetInOut(uint64_t &inbytes, uint64_t &outbytes,
  const std::string &netIface, bool ignored) const {

    inbytes = outbytes = 0;

    const int mib_ifl[] = { CTL_NET, PF_ROUTE, 0, 0, NET_RT_IFLIST, 0 };
    const size_t mibsize = sizeof(mib_ifl) / sizeof(mib_ifl[0]);

    size_t size;
    if ( sysctl(mib_ifl, mibsize, NULL, &size, NULL, 0) < 0 )
        logFatal << "sysctl() failed." << std::endl;

    std::vector<char> bufv(size, 0);
    if ( sysctl(mib_ifl, mibsize, bufv.data(), &size, NULL, 0) < 0 )
        logFatal << "sysctl() failed." << std::endl;

    const char *bufp = bufv.data();
    const struct rt_msghdr *rtm =
        reinterpret_cast<const struct rt_msghdr *>(bufp);

    for ( ; bufp < bufv.data() + size ; bufp += rtm->rtm_msglen) {

        rtm = reinterpret_cast<const struct rt_msghdr *>(bufp);
        if (rtm->rtm_version != RTM_VERSION)
            continue;

        if (rtm->rtm_type == RTM_IFINFO) {
            const struct if_msghdr *ifm =
                reinterpret_cast<const struct if_msghdr *>(bufp);
            const struct sockaddr_dl *sdl =
                reinterpret_cast<const struct sockaddr_dl *>(ifm + 1); // voodoo

            if (sdl->sdl_family != AF_LINK)
                continue;

            std::string ifname(sdl->sdl_data, 0, sdl->sdl_nlen);
            if (ifskip(ifname, netIface, ignored))
                continue;

            inbytes += ifm->ifm_data.ifi_ibytes;;
            outbytes += ifm->ifm_data.ifi_obytes;

            logDebug << "iface (in, out): " << ifname << " ("
                     << ifm->ifm_data.ifi_ibytes << ", "
                     << ifm->ifm_data.ifi_obytes << ")" << std::endl;
        }
    }
}
#endif


#if defined(XOSVIEW_DFBSD) || defined(XOSVIEW_FREEBSD)
void NetMeter::getNetInOut(uint64_t &inbytes, uint64_t &outbytes,
  const std::string &netIface, bool ignored) const {

    // Lookup the mib for sysctl only once.
    static SysCtl ifcount_sc("net.link.generic.system.ifcount");
    // net.link.generic.ifdata.N.general
    // Can't look this one up by name.  So, directly monkey with the mib.
    static SysCtl ifdata_sc("net.link.generic.ifdata");
    static bool first = true;
    if (first) {
        first = false;
        ifdata_sc.mib().push_back(1);  // Place holder for N (first == 1).
        ifdata_sc.mib().push_back(IFDATA_GENERAL);
    }

    inbytes = outbytes = 0;

    int ifcount = 0;
    if (!ifcount_sc.get(ifcount))
        logFatal << "sysctl " << ifcount_sc.id() << " failed." << std::endl;

    // rows start with index == 1 not 0!
    for (int i = 1 ; i <= ifcount ; i++) {
        struct ifmibdata ifmd;
        ifdata_sc.mib()[4] = i; // Set N in the mib
        if (!ifdata_sc.get(ifmd)) {
            logFatal << "sysctl(" << ifdata_sc.id() << ") failed." << std::endl;
        }
        if (!(ifmd.ifmd_flags & IFF_UP))
            continue;

        std::string ifname(ifmd.ifmd_name);
        if (ifskip(ifname, netIface, ignored))
            continue;

        inbytes  += ifmd.ifmd_data.ifi_ibytes;
        outbytes += ifmd.ifmd_data.ifi_obytes;

        logDebug << "name: " << ifname << " - "
                 << "out: " << ifmd.ifmd_data.ifi_obytes << ", "
                 << "in: " << ifmd.ifmd_data.ifi_ibytes << std::endl;
    }
}
#endif
