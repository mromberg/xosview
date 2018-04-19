//
//  Copyright (c) 1994, 1995, 2002, 2006, 2015, 2016, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  Modifications to support dynamic addresses by:
//    Michael N. Lipp (mnl@dtro.e-technik.th-darmstadt.de)
//
//  This file may be distributed under terms of the GPL
//

#include "nfsmeter.h"

#include <fstream>


static const char * const NFSSVCSTAT = "/proc/net/rpc/nfsd";
static const char * const NFSCLTSTAT = "/proc/net/rpc/nfs";


NFSMeter::NFSMeter(const std::string &name, size_t nfields,
  const std::string &fields, const std::string &statfile)
    : FieldMeterGraph(nfields, name, fields),
      _statname(name), _statfile(statfile) {
}


void NFSMeter::checkResources(const ResDB &rdb) {
    FieldMeterGraph::checkResources(rdb);
}


NFSDStats::NFSDStats(void)
    : NFSMeter("NFSD", 4, "BAD/UDP/TCP/IDLE", NFSSVCSTAT) {
    timerStart();
}


void NFSDStats::checkResources(const ResDB &rdb) {
    NFSMeter::checkResources(rdb);

    setfieldcolor(0, rdb.getColor( "NFSDStatBadCallsColor"));
    setfieldcolor(1, rdb.getColor( "NFSDStatUDPColor"));
    setfieldcolor(2, rdb.getColor( "NFSDStatTCPColor"));
    setfieldcolor(3, rdb.getColor( "NFSDStatIdleColor"));
}


void NFSDStats::checkevent(void) {

    std::ifstream ifs(_statfile);

    if (!ifs)
        return;

    std::fill(_fields.begin(), _fields.end(), 0); // network activity
    timerStop();

    std::string name;
    unsigned long netcnt = 0, netudpcnt = 0, nettcpcnt = 0, nettcpconn = 0;
    unsigned long calls = 0, badcalls = 0;
    size_t found = 0;
    while (!ifs.eof() && found != 2) {
        std::string buf;
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

    float t = 1000000.0 / etimeUsecs();
    if (t < 0)
        t = 0.1;

    _maxpackets = std::max(netcnt, calls) - _lastNetCnt;
    if (_maxpackets == 0)
        _maxpackets = netcnt;
    else {
        _fields[0] = (badcalls - _lastBad) * t;
        _fields[1] = (netudpcnt - _lastUdp) * t;
        _fields[2] = (nettcpcnt - _lastTcp) * t;
    }

    _total = _fields[0] + _fields[1] + _fields[2];
    if (_total > _maxpackets)
        _fields[3] = 0;
    else {
        _total = _maxpackets;
        _fields[3] = _total - _fields[0] - _fields[1] - _fields[2];
    }

    if (_total)
        setUsed(_fields[0] + _fields[1] + _fields[2], _total);

    timerStart();

    _lastNetCnt = std::max(netcnt, calls);
    _lastTcp = nettcpcnt;
    _lastUdp = netudpcnt;
    _lastBad = badcalls;
}


NFSStats::NFSStats(void)
    : NFSMeter("NFS", 4, "RETRY/AUTH/CALL/IDLE", NFSCLTSTAT) {
    timerStart();
}


void NFSStats::checkResources(const ResDB &rdb) {
    NFSMeter::checkResources(rdb);

    setfieldcolor( 0, rdb.getColor("NFSStatReTransColor"));
    setfieldcolor( 1, rdb.getColor("NFSStatAuthRefrshColor"));
    setfieldcolor( 2, rdb.getColor("NFSStatCallsColor"));
    setfieldcolor( 3, rdb.getColor("NFSStatIdleColor"));
}


void NFSStats::checkevent(void) {

    std::ifstream ifs(_statfile);
    if (!ifs)
        return;

    std::fill(_fields.begin(), _fields.end(), 0);
    timerStop();

    std::string name;
    unsigned long calls = 0, retrns = 0, authrefresh = 0, _maxpackets = 0;
    while (!ifs.eof()) {
        std::string buf;
        std::getline(ifs, buf);
        if (buf.substr(0, 3) != "rpc")
            continue;
        std::istringstream iss(buf);
        iss >> name >> calls >> retrns >> authrefresh;
        break;
    }

    float t = 1000000.0 / etimeUsecs();
    if (t < 0)
        t = 0.1;

    _maxpackets = calls - _lastcalls;
    if (_maxpackets == 0)
        _maxpackets = calls;
    else {
        _fields[2] = (calls - _lastcalls) * t;
        _fields[1] = (authrefresh - _lastauthrefresh) * t;
        _fields[0] = (retrns - _lastretrns) * t;
    }

    _total = _fields[0] + _fields[1] + _fields[2];
    if (_total > _maxpackets)
        _fields[3] = 0;
    else {
        _total = _maxpackets;
        _fields[3] = _total - _fields[2] - _fields[1] - _fields[0];
    }

    if (_total)
        setUsed(_fields[0] + _fields[1] + _fields[2], _total);

    timerStart();

    _lastcalls = calls;
    _lastretrns = retrns;
    _lastauthrefresh = authrefresh;
}
