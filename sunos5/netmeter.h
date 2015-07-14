//
//  Copyright (c) 1994, 1995, 2015
//  by Mike Romberg (mike-romberg@comcast.net)
//
//  This file may be distributed under terms of the GPL
//
#ifndef NETMETER_H
#define NETMETER_H

#include "fieldmetergraph.h"

#include <kstat.h>

#define NNETS 100
#define GUESS_MTU 1500

class Host;

class NetMeter : public FieldMeterGraph {
public:
    NetMeter(XOSView *parent, kstat_ctl_t *_kc, float max);
    ~NetMeter( void );

    virtual std::string name( void ) const { return "NetMeter"; }
    void checkevent( void );

    void checkResources( void );
protected:
    float maxpackets_;

private:
    long long _lastBytesIn, _lastBytesOut;

    void adjust(void);
    int nnet;
    kstat_ctl_t *kc;
    kstat_t *nnets[NNETS];
    int packetsize[NNETS];
};

#endif
