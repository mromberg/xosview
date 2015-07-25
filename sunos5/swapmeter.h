//
//  Copyright (c) 1999, 2015
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//

#ifndef SWAPMETER_H
#define SWAPMETER_H

#include "fieldmetergraph.h"




class SwapMeter : public FieldMeterGraph {
public:
    SwapMeter(XOSView *parent);
    ~SwapMeter(void);

    virtual std::string name(void) const { return "SwapMeter"; }
    void checkevent(void);
    void checkResources(void);

protected:
    void getswapinfo(void);

private:
    size_t pagesize;
};


#endif
