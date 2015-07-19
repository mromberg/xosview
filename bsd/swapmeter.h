//
//  Copyright (c) 1995, 1996, 1997-2002, 2015 by Brian Grayson (bgrayson@netbsd.org)
//
//  This file was written by Brian Grayson for the NetBSD and xosview
//    projects.
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//

#ifndef SWAPMETER_H
#define SWAPMETER_H

#include "fieldmetergraph.h"


class SwapMeter : public FieldMeterGraph {
public:
    SwapMeter( XOSView *parent );
    ~SwapMeter( void );

    virtual std::string name( void ) const { return "SwapMeter"; }
    void checkevent( void );
    void checkResources( void );

protected:
    void getswapinfo( void );
};

#endif
