//
//  Copyright (c) 1999, 2015
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//

#ifndef SWAPMETER_H
#define SWAPMETER_H

#include "cswapmeter.h"




class SwapMeter : public ComSwapMeter {
public:
    SwapMeter(void);

protected:
    virtual std::pair<uint64_t, uint64_t> getswapinfo( void );

private:
    const size_t _pagesize;
};


#endif
