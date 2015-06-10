//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef _NETMETER_H_
#define _NETMETER_H_

#include "fieldmetergraph.h"
#include "timer.h"

class Host;

class NetMeter : public FieldMeterGraph {
public:
    NetMeter(XOSView *parent, float max);
    ~NetMeter( void );

    std::string name( void ) const { return "NetMeter"; }
    void checkevent( void );

    void checkResources( void );
protected:
    float maxpackets_;
    std::string netIface_;

private:
    int _ipsock;
    Timer _timer;
    unsigned long long _lastBytesIn, _lastBytesOut;
    const char *_netfilename;
    bool _usechains;
    int _bytesInDev;

    void adjust(void);
    void checkOSVersion(void);

    void checkeventOld(void);
    void checkeventNew(void);
};

#endif
