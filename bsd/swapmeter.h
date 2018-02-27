//
//  Copyright (c) 1995, 1996, 1997-2002, 2015, 2016, 2018
//  by Brian Grayson (bgrayson@netbsd.org)
//
//  This file was written by Brian Grayson for the NetBSD and xosview
//    projects.
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//

#ifndef swapmeter_h
#define swapmeter_h

#include "cswapmeter.h"



class SwapMeter : public ComSwapMeter {
public:
    SwapMeter(void);

protected:
    virtual std::pair<uint64_t, uint64_t> getswapinfo(void) override;

private:
    void getSwapInfo(uint64_t &total, uint64_t &used) const;
};


#endif
