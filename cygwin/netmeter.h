//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef NETMETER_H
#define NETMETER_H

#include "perfcount.h"
#include "fieldmetergraph.h"



class NetMeter : public FieldMeterGraph {
public:
    NetMeter(XOSView *parent);
    ~NetMeter( void );

    std::string name( void ) const { return "NetMeter"; }
    void checkevent( void );

    void checkResources(const ResDB &rdb);

    // first=total in second=total out
    // There may be some new C++ way to make this
    // private.  But if so, I don't know it
    typedef std::pair<unsigned long long, unsigned long long> netpair;

private:
    float _maxBandwidth;  // in bytes/sec
    PerfQuery _query;
    size_t _inIndex;

    netpair getStats(void);
    void add(const std::vector<std::string> &adapters,
      const std::vector<std::string> &plist);
};


#endif
