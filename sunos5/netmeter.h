//
//  Copyright (c) 1994, 1995, 2015
//  by Mike Romberg (mike-romberg@comcast.net)
//
//  This file may be distributed under terms of the GPL
//
#ifndef NETMETER_H
#define NETMETER_H

#include "fieldmetergraph.h"


// To keep the header in the .cc file
struct kstat_ctl;
typedef kstat_ctl kstat_ctl_t;
struct kstat;
typedef kstat kstat_t;

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
    size_t nnet;
    kstat_ctl_t *kc;
    std::vector<kstat_t *> nnets;
    std::vector<int> packetsize;
};

#endif
