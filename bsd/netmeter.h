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

#ifndef NETMETER_H
#define NETMETER_H

#include "fieldmetergraph.h"



class NetMeter : public FieldMeterGraph {
public:
    NetMeter( XOSView *parent, double max );
    ~NetMeter( void );

    virtual std::string name( void ) const { return "NetMeter"; }
    void checkevent( void );
    void checkResources(const ResDB &rdb);

private:
    uint64_t lastBytesIn_, lastBytesOut_;
    double netBandwidth_;
    std::string netIface_;
    bool ignored_;

    void getstats(void);
};


#endif
