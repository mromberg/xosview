//
//  Copyright (c) 1994, 1995, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#ifndef NETMETER_H
#define NETMETER_H

#include "fieldmetergraph.h"
#include "kstats.h"

#include <net/if.h>



class NetMeter : public FieldMeterGraph {
public:
    NetMeter( XOSView *parent, kstat_ctl_t *kc, float max );
    ~NetMeter( void );

    virtual std::string name( void ) const { return "NetMeter"; }
    void checkevent( void );
    void checkResources( void );

protected:
    void getnetstats( void );

private:
    float _maxpackets;
    uint64_t _lastBytesIn, _lastBytesOut;
    kstat_ctl_t *_kc;
    KStatList *_nets;
    std::string _netIface;
    bool _ignored;
    struct lifreq _lfr;
    int _socket;
};


#endif
