//
//  NetBSD port:
//  Copyright (c) 1995,1996,1997,2015 Brian Grayson(bgrayson@netbsd.org)
//
//  This file was written by Brian Grayson for the NetBSD and xosview
//    projects.
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//

#ifndef DISKMETER_H
#define DISKMETER_H

#include "fieldmetergraph.h"



class DiskMeter : public FieldMeterGraph {
public:
    DiskMeter( XOSView *parent, double max );
    ~DiskMeter( void );

    virtual std::string name( void ) const { return "DiskMeter"; }
    void checkevent( void );
    void checkResources(const ResDB &rdb);

private:
#ifndef HAVE_DEVSTAT
    uint64_t prevreads_, prevwrites_;
#endif
    double maxBandwidth_;

    void getstats( void );
};


#endif
