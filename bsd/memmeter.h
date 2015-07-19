//
//  Copyright (c) 1994, 1995, 2015 by Mike Romberg ( romberg@fsl.noaa.gov )
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

#ifndef MEMMETER_H
#define MEMMETER_H

#include "fieldmetergraph.h"


class MemMeter : public FieldMeterGraph {
public:
    MemMeter( XOSView *parent );
    ~MemMeter( void );

    virtual std::string name( void ) const { return "MemMeter"; }
    void checkevent( void );
    void checkResources( void );

protected:
    void getmeminfo( void );

private:
    std::vector<uint64_t> meminfo_;
};

#endif
