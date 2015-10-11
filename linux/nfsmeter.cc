//
//  Copyright (c) 1994, 1995, 2002, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  Modifications to support dynamic addresses by:
//    Michael N. Lipp (mnl@dtro.e-technik.th-darmstadt.de)
//
//  This file may be distributed under terms of the GPL
//


#include "nfsmeter.h"

#include <fstream>
#include <cerrno>
#include <iostream>
#include <iomanip>
#include <algorithm>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#if defined(GNULIBC) || defined(__GLIBC__)
#include <net/if.h>
#else
#include <linux/if.h>
#endif
#include <netinet/in.h>


static const char * const NFSSVCSTAT = "/proc/net/rpc/nfsd";
static const char * const NFSCLTSTAT = "/proc/net/rpc/nfs";


NFSMeter::NFSMeter(XOSView *parent, const std::string &name, int nfields,
  const std::string &fields, const std::string &statfile)
    : FieldMeterGraph( parent, nfields, name, fields ),
      _statname(name), _statfile(statfile) {
}


NFSMeter::~NFSMeter( void ){
}


void NFSMeter::checkResources(const ResDB &rdb){
    FieldMeterGraph::checkResources(rdb);
}

NFSDStats::NFSDStats(XOSView *parent)
    : NFSMeter(parent, "NFSD", 4, "BAD/UDP/TCP/IDLE", NFSSVCSTAT ){
    starttimer();
}


NFSDStats::~NFSDStats( void ) {
}


void NFSDStats::checkResources(const ResDB &rdb){
    NFSMeter::checkResources(rdb);

    setfieldcolor( 0, rdb.getColor( "NFSDStatBadCallsColor" ) );
    setfieldcolor( 1, rdb.getColor( "NFSDStatUDPColor" ) );
    setfieldcolor( 2, rdb.getColor( "NFSDStatTCPColor" ) );
    setfieldcolor( 3, rdb.getColor( "NFSDStatIdleColor" ) );

    setUsedFormat (rdb.getResource("NFSDStatUsedFormat"));
    decayUsed(rdb.isResourceTrue("NFSDStatUsedDecay"));
    //useGraph_ = 1;
    //dodecay_ = 1;
    //setUsedFormat ("autoscale");
    //setUsedFormat ("percent");
}


void NFSDStats::checkevent(void) {
    std::string buf, name;
    unsigned long netcnt, netudpcnt, nettcpcnt, nettcpconn;
    unsigned long calls, badcalls;
    int found;

    std::ifstream ifs(_statfile.c_str());

    if (!ifs) {
        return;
    }

    fields_[0] = fields_[1] = fields_[2] = 0;  // network activity
    stoptimer();

    name[0] = '\0';
    found = 0;
    while (!ifs.eof() && found != 2) {
        std::getline(ifs, buf);
        if (buf.substr(0, 3) == "net") {
            std::istringstream iss(buf);
            iss >> name >> netcnt >> netudpcnt >> nettcpcnt >> nettcpconn;
            found++;
        }
        if (buf.substr(0, 3) == "rpc") {
            std::istringstream iss(buf);
            iss >> name >> calls >> badcalls;
            found++;
        }
    }

    float t = 1000000.0 / usecs();

    if (t < 0)
        t = 0.1;

    maxpackets_ = std::max(netcnt, calls) - _lastNetCnt;
    if (maxpackets_ == 0) {
        maxpackets_ = netcnt;
    }
    else {
        fields_[0] = (badcalls - _lastBad) * t;
        fields_[1] = (netudpcnt - _lastUdp) * t;
        fields_[2] = (nettcpcnt - _lastTcp) * t;
    }

    total_ = fields_[0] + fields_[1] + fields_[2];
    if (total_ > maxpackets_)
        fields_[3] = 0;
    else {
        total_ = maxpackets_;
        fields_[3] = total_ - fields_[0] - fields_[1] - fields_[2];
    }

    if (total_)
        setUsed(fields_[0] + fields_[1] + fields_[2], total_);

    starttimer();

    _lastNetCnt = std::max(netcnt, calls);
    _lastTcp = nettcpcnt;
    _lastUdp = netudpcnt;
    _lastBad = badcalls;
}


NFSStats::NFSStats(XOSView *parent)
    : NFSMeter(parent, "NFS", 4, "RETRY/AUTH/CALL/IDLE", NFSCLTSTAT ){
    starttimer();
}


NFSStats::~NFSStats( void ) {
}


void NFSStats::checkResources(const ResDB &rdb){
    NFSMeter::checkResources(rdb);

    setfieldcolor( 0, rdb.getColor( "NFSStatReTransColor" ) );
    setfieldcolor( 1, rdb.getColor( "NFSStatAuthRefrshColor" ) );
    setfieldcolor( 2, rdb.getColor( "NFSStatCallsColor" ) );
    setfieldcolor( 3, rdb.getColor( "NFSStatIdleColor" ) );

    setUsedFormat (rdb.getResource("NFSStatUsedFormat"));
    //setUsedFormat ("autoscale");
    //setUsedFormat ("percent");
}


void NFSStats::checkevent(void) {
    std::string buf, name;
    unsigned long calls, retrns, authrefresh, maxpackets_;

    std::ifstream ifs(_statfile.c_str());

    if (!ifs) {
        return;
    }

    fields_[0] = fields_[1] = fields_[2] = 0;
    stoptimer();

    name[0] = '\0';
    while (!ifs.eof()) {
        std::getline(ifs, buf);
        if (buf.substr(0, 3) != "rpc")
            continue;
        std::istringstream iss(buf);
        iss >> name >> calls >> retrns >> authrefresh;
        break;
    }

    float t = 1000000.0 / usecs();

    if (t < 0)
        t = 0.1;

    maxpackets_ = calls - _lastcalls;
    if (maxpackets_ == 0) {
        maxpackets_ = calls;
    }
    else {
        fields_[2] = (calls - _lastcalls) * t;
        fields_[1] = (authrefresh - _lastauthrefresh) * t;
        fields_[0] = (retrns - _lastretrns) * t;
    }

    total_ = fields_[0] + fields_[1] + fields_[2];
    if (total_ > maxpackets_)
        fields_[3] = 0;
    else {
        total_ = maxpackets_;
        fields_[3] = total_ - fields_[2] - fields_[1] - fields_[0];
    }

    if (total_)
        setUsed(fields_[0] + fields_[1] + fields_[2], total_);

    starttimer();

    _lastcalls = calls;
    _lastretrns = retrns;
    _lastauthrefresh = authrefresh;
}
