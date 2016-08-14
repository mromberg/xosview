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

#include "swapmeter.h"
#include "sctl.h"

#if defined(XOSVIEW_NETBSD) || defined(XOSVIEW_OPENBSD)
#include <sys/swap.h>
#include <unistd.h>
#endif


SwapMeter::SwapMeter( void )
    : ComSwapMeter() {
}


std::pair<uint64_t, uint64_t> SwapMeter::getswapinfo( void ) {
    uint64_t total = 0, used = 0;

    getSwapInfo(total, used);

    return std::make_pair(total, total - used);
}


#if defined(XOSVIEW_DFBSD)
void SwapMeter::getSwapInfo(uint64_t &total, uint64_t &used) {
    static SysCtl swsize_sc("vm.swap_size");
    static SysCtl swanon_sc("vm.swap_anon_use");
    static SysCtl swcache_sc("vm.swap_cache_use");

    size_t pagesize = getpagesize();
    int ssize = 0;
    int anon = 0;
    int cache = 0;

    if (!swsize_sc.get(ssize))
        logFatal << "sysctl(" << swsize_sc.id() << ") failed." << std::endl;
    if (!swanon_sc.get(anon))
        logFatal << "sysctl(" << swanon_sc.id() << ") failed." << std::endl;
    if (!swcache_sc.get(cache))
        logFatal << "sysctl(" << swcache_sc.id() << ") failed." << std::endl;

    total = ssize * pagesize;
    used = (anon + cache) * pagesize;
}
#endif


#if defined(XOSVIEW_FREEBSD)
void SwapMeter::getSwapInfo(uint64_t &total, uint64_t &used) {

    static SysCtl nswaps_sc("vm.nswapdev");
    static SysCtl swapinfo_sc("vm.swap_info");
    if (swapinfo_sc.mib().size() == 2)
        swapinfo_sc.mib().push_back(0);

    int nswaps = 0;
    if (!nswaps_sc.get(nswaps))
        logFatal << "sysctl(" << nswaps_sc.id() << ") failed." << std::endl;

    // This thing is neither documented or even defined in a system
    // header.  We use it here because it is not KVM.  If it breaks then
    // FreeBSD does not get a swap meter until it has some sane way
    // to get the stats.
    //
    // Instead of the following an array of five ints will be used.
    //
    // struct xswdev {
    //     u_int   xsw_version;
    //     dev_t   xsw_dev;
    //     int     xsw_flags;
    //     int     xsw_nblks;
    //     int     xsw_used;
    // };

    size_t psize = getpagesize();

    std::vector<int> xswd(5, 0);
    for (int i = 0 ; i < nswaps ; i++) {
        swapinfo_sc.mib()[2] = i;
        if (!swapinfo_sc.get(xswd))
            logFatal << "sysctl(" << swapinfo_sc.id() << ") failed."
                     << std::endl;

        total += xswd[3] * psize;
        used += xswd[4] * psize;
    }
}
#endif


#if defined(XOSVIEW_NETBSD) || defined(XOSVIEW_OPENBSD)
void SwapMeter::getSwapInfo(uint64_t &total, uint64_t &used) {
    //  This code is based on a patch sent in by Scott Stevens
    //  (s.k.stevens@ic.ac.uk, at the time).
    int rnswap, nswap = swapctl(SWAP_NSWAP, 0, 0);
    total = used = 0;

    if (nswap < 1)  // no swap devices on
        return;

    std::vector<struct swapent> sep(nswap);

    rnswap = swapctl(SWAP_STATS, (void *)sep.data(), nswap);
    if (rnswap < 0)
        logFatal << "BSDGetSwapInfo(): getting SWAP_STATS failed" << std::endl;
    if (nswap != rnswap)
        logProblem << "SWAP_STATS gave different value than SWAP_NSWAP "
                   << "(nswap=" << nswap << " versus "
                   << "rnswap=" << rnswap << std::endl;

    // block size is that of underlying device, *usually* 512 bytes
    const int bsize = 512;
    for (size_t i = 0 ; i < (size_t)rnswap ; i++) {
        total += (uint64_t)sep[i].se_nblks * bsize;
        used += (uint64_t)sep[i].se_inuse * bsize;
    }
}
#endif
