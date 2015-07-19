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
#include "timer.h"

class Host;

class NetMeter : public FieldMeterGraph {
public:
    NetMeter(XOSView *parent, float max);
    ~NetMeter( void );

    virtual std::string name( void ) const { return "NetMeter"; }
    void checkevent( void );

    void checkResources( void );

    void BSDGetNetInOut (long long * inbytes, long long * outbytes);

    static bool checkPerms(void);

private:
    float netBandwidth_;
    //  NetBSD:  Use long long, so we won't run into problems after 4 GB
    //  has been transferred over the net!
    long long _lastBytesIn, _lastBytesOut;

    //  Did the meter initialize properly?
    bool kernelHasStats_;
    std::string netIface_;

    void adjust(void);
};

#endif
