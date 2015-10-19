//
//  Copyright (c) 1999, 2015
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//
#include "swapmeter.h"


#include <unistd.h>
#include <limits.h>
#include <sys/swap.h>



SwapMeter::SwapMeter(void)
    : ComSwapMeter(), _pagesize(sysconf(_SC_PAGESIZE)) {
}


std::pair<uint64_t, uint64_t> SwapMeter::getswapinfo( void ) {

    std::pair<uint64_t, uint64_t> rval(0, 0);

    int numswap = swapctl(SC_GETNSWP, NULL);
    if (numswap < 0)
        logFatal << "Can not determine number of swap spaces." << std::endl;

    if (numswap > 0) {

        std::vector<swaptbl_t> swaps(numswap + 1);
        std::vector<std::vector<char> > names(numswap + 1,
          std::vector<char>(PATH_MAX, '\0'));

        for (int i = 0; i <= numswap; i++)
            swaps[0].swt_ent[i].ste_path = names[i].data();

        swaps[0].swt_n = numswap + 1;
        int stcount = swapctl(SC_LIST, swaps.data());
        if ( stcount < 0)
            logFatal << "Can not get list of swap spaces." << std::endl;

        // This is as wierd as it looks.  The
        // first element now has an array of pointers
        // into the rest of swaps (I think [but not like a Sun engineer])
        if (stcount <= numswap) {
            for (int i = 0 ; i < stcount ; i++) {
                rval.first += swaps[0].swt_ent[i].ste_pages;
                rval.second += swaps[0].swt_ent[i].ste_free;
                logDebug << swaps[0].swt_ent[i].ste_path << ": "
                         << swaps[0].swt_ent[i].ste_pages * (_pagesize / 1024)
                         << " kB ("
                         << swaps[0].swt_ent[i].ste_free * (_pagesize / 1024)
                         << " kB free)" << std::endl;
            }
        }
        else {
            // more were added.  Get them on the next pass
        }
    }

    rval.first *= _pagesize;
    rval.second *= _pagesize;

    return rval;
}
