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



SwapMeter::SwapMeter(XOSView *parent)
    : FieldMeterGraph(parent, 2, "SWAP", "USED/FREE"),
      pagesize(sysconf(_SC_PAGESIZE)) {
}


SwapMeter::~SwapMeter(void) {
}


void SwapMeter::checkResources(void) {

    FieldMeterGraph::checkResources();

    setfieldcolor(0, parent_->getResource("swapUsedColor"));
    setfieldcolor(1, parent_->getResource("swapFreeColor"));
    priority_ = util::stoi(parent_->getResource("swapPriority"));
    dodecay_ = parent_->isResourceTrue("swapDecay");
    useGraph_ = parent_->isResourceTrue("swapGraph");
    setUsedFormat(parent_->getResource("swapUsedFormat"));
    decayUsed(parent_->isResourceTrue("swapUsedDecay"));
}


void SwapMeter::checkevent(void) {
    getswapinfo();
    drawfields(parent_->g());
}


void SwapMeter::getswapinfo(void) {
    int numswap = swapctl(SC_GETNSWP, NULL);

    total_ = fields_[0] = fields_[1] = 0;

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
                total_ += swaps[0].swt_ent[i].ste_pages;
                fields_[1] += swaps[0].swt_ent[i].ste_free;
                logDebug << swaps[0].swt_ent[i].ste_path << ": "
                         << swaps[0].swt_ent[i].ste_pages * (pagesize / 1024)
                         << " kB ("
                         << swaps[0].swt_ent[i].ste_free * (pagesize / 1024)
                         << " kB free)" << std::endl;
            }
        }
        else {
            // more were added.  Get them on the next pass
        }
    }

    fields_[0] = total_ - fields_[1];
    setUsed(fields_[0] * pagesize, total_ * pagesize);
}
