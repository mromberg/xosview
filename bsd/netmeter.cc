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
#include "netmeter.h"

#if defined(XOSVIEW_FREEBSD) || defined(XOSVIEW_DFBSD)
#include <sys/sysctl.h>
#include <net/if.h>
#include <net/if_mib.h>
#endif


NetMeter::NetMeter( void )
    : ComNetMeter(),
      _lastBytesIn(0), _lastBytesOut(0),
      _netIface("False"), _ignored(false) {
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
        IntervalTimerStart();
        return std::make_pair(0.0, 0.0);
    }

    uint64_t nowBytesIn, nowBytesOut;

    IntervalTimerStop();
    getNetInOut(nowBytesIn, nowBytesOut, _netIface, _ignored);
    double t = (1.0) / IntervalTimeInSecs();
    IntervalTimerStart();

    std::pair<float, float> rval((nowBytesIn - _lastBytesIn) * t,
      (nowBytesOut - _lastBytesOut) * t);
    _lastBytesIn = nowBytesIn;
    _lastBytesOut = nowBytesOut;

    return rval;
}


#if defined(XOSV_NETBSD_NET_IOCTL)
// -----------------------------------------------------------
// Based on a code snippet from Martin Husemann on the NetBSD
// forums.
// -----------------------------------------------------------
void NetMeter::getNetInOut(uint64_t &inbytes, uint64_t &outbytes,
  const std::string &netIface, bool ignored) const {

    inbytes = outbytes = 0;
    struct if_nameindex *iflist = if_nameindex();
    int s = socket(AF_LOCAL, SOCK_DGRAM, 0);

    for (struct if_nameindex *p = iflist; p->if_index > 0; p++) {
        struct ifdatareq ifdr;
        memset(&ifdr, 0, sizeof(ifdr));
        std::string p_if_name(p->if_name);
        memcpy(ifdr.ifdr_name, p_if_name.c_str(), p_if_name.length());
        if (ioctl(s, SIOCGIFDATA, &ifdr) == -1) {
            logFatal << "ioctl(SIOCGIFDATA) failed for: " << p->if_name
                     << std::endl;
        }
        std::string ifname(ifdr.ifdr_name);
        if (netIface != "False") {
            if ((!ignored && netIface != ifname)
              || (ignored && netIface == ifname))
                continue;
        }
        const struct if_data *ifi = &ifdr.ifdr_data;
        inbytes += ifi->ifi_ibytes;
        outbytes += ifi->ifi_obytes;

        logDebug << ifname
                 << " in: " << ifi->ifi_ibytes
                 << " out: " << ifi->ifi_obytes << std::endl;
    }

    close(s);
    if_freenameindex(iflist);
}
#endif


#if defined(XOSVIEW_OPENBSD)
void NetMeter::getNetInOut(uint64_t &inbytes, uint64_t &outbytes,
  const std::string &netIface, bool ignored) const {

    inbytes = outbytes = 0;
    const int mib_ifl[] = { CTL_NET, PF_ROUTE, 0, 0, NET_RT_IFLIST, 0 };

    size_t size;
    if ( sysctl(mib_ifl, ASIZE(mib_ifl), NULL, &size, NULL, 0) < 0 )
        logFatal << "BSDGetNetInOut(): sysctl 1 failed" << std::endl;

    std::vector<char> bufv(size, 0);
    if ( sysctl(mib_ifl, ASIZE(mib_ifl), bufv.data(), &size, NULL, 0) < 0 )
        logFatal << "BSDGetNetInOut(): sysctl 2 failed" << std::endl;

    struct if_msghdr *ifm = (struct if_msghdr *)bufv.data();

    char *bufp = bufv.data();
    struct rt_msghdr *rtm = (struct rt_msghdr *)bufp;

    for ( ; bufp < bufv.data() + size ; bufp += rtm->rtm_msglen) {

        rtm = (struct rt_msghdr *)bufp;
        if (rtm->rtm_version != RTM_VERSION)
            continue;

        if (rtm->rtm_type == RTM_IFINFO) {
            ifm = (struct if_msghdr *)bufp;
            struct sockaddr_dl *sdl = (struct sockaddr_dl *)(ifm + 1); // vodoo?
            if (sdl->sdl_family != AF_LINK)
                continue;
            std::string ifname(sdl->sdl_data, 0, sdl->sdl_nlen);

            if (netIface != "False") {
                if ( (!ignored && netIface != ifname)
                  || (ignored && netIface == ifname) )
                    continue;
            }

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

    inbytes = outbytes = 0;
    int ifcount = 0;
    size_t ifcsize = sizeof(ifcount);
    if (sysctlbyname("net.link.generic.system.ifcount", &ifcount, &ifcsize,
        NULL, 0) < 0) {
        logFatal << "sysctlbyname(net.link.generic.system.ifcount) failed.\n";
    }

    // rows start with index == 1 not 0!
    int mib[] = {CTL_NET, PF_LINK, NETLINK_GENERIC, IFMIB_IFDATA,
                 1, IFDATA_GENERAL};
    struct ifmibdata ifmd;
    size_t size = sizeof(ifmd);

    for (int i = 1 ; i <= ifcount ; i++) {
        mib[4] = i;
        if (sysctl(mib, 6, &ifmd, &size, NULL, 0) < 0) {
            logFatal << "sysctl(CTL_NET) failed.\n";
        }
        if (!(ifmd.ifmd_flags & IFF_UP))
            continue;

        std::string ifname(ifmd.ifmd_name);
        if (netIface != "False") {
            if ( (!ignored && netIface != ifname)
              || (ignored && netIface == ifname) )
                continue;
        }

        inbytes  += ifmd.ifmd_data.ifi_ibytes;
        outbytes += ifmd.ifmd_data.ifi_obytes;

        logDebug << "name: " << ifname << " - "
                 << "out: " << ifmd.ifmd_data.ifi_obytes << ", "
                 << "in: " << ifmd.ifmd_data.ifi_ibytes << std::endl;
    }
}
#endif
