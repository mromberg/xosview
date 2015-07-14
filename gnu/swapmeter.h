//
//  Copyright (c) 1994, 1995, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//  2007 by Samuel Thibault ( samuel.thibault@ens-lyon.org )
//
//  This file may be distributed under terms of the GPL
//
#ifndef SWAPMETER_H
#define SWAPMETER_H

#include "fieldmetergraph.h"

extern "C" {
#include <mach/mach_types.h>
#include <mach/default_pager_types.h>
}

class SwapMeter : public FieldMeterGraph {
public:
    SwapMeter( XOSView *parent );
    ~SwapMeter( void );

    virtual std::string name( void ) const { return "SwapMeter"; }
    void checkevent( void );

    void checkResources( void );

protected:
  void getswapinfo( void );

private:
    struct default_pager_info def_pager_info;
    mach_port_t def_pager;
};

#endif
