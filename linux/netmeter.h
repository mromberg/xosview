//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef NETMETER_H
#define NETMETER_H

#include "fieldmetergraph.h"
#include "timer.h"

class Host;

class NetMeter : public FieldMeterGraph {
public:
    NetMeter(XOSView *parent);
    ~NetMeter( void );

    std::string name( void ) const { return "NetMeter"; }
    void checkevent( void );

    void checkResources( void );

    // first=total in second=total out
    // There may be some new C++ way to make this
    // private.  But if so, I don't know it
    typedef std::pair<unsigned long long, unsigned long long> netpair;

private:
    Timer _timer;
    netpair _last;
    float _maxBandwidth;  // in bytes/sec
    std::string _netfilename;

    netpair getStats(void);
};

#endif
