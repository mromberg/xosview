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

#ifndef PAGEMETER_H
#define PAGEMETER_H

#include "fieldmetergraph.h"


class PageMeter : public FieldMeterGraph {
public:
    PageMeter( XOSView *parent, double total );
    ~PageMeter( void );

    virtual std::string name( void ) const { return "PageMeter"; }
    void checkevent( void );
    void checkResources( void );

protected:
    void getpageinfo( void );

private:
    std::vector<uint64_t> previnfo_;
};

#endif
