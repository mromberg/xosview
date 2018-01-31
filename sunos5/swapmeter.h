//
//  Copyright (c) 1999, 2015, 2018
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
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
    const size_t _pagesize;
};


#endif
