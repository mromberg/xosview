//
//
//  This file may be distributed under terms of the GPL
//
#ifndef NFSMETER_H
#define NFSMETER_H

#include "fieldmetergraph.h"
#include "timer.h"

class Host;

class NFSMeter : public FieldMeterGraph {
public:
    NFSMeter(
        const std::string &name,
        int   nfields,
        const std::string &files,
        const std::string &statfile);
    ~NFSMeter( void );

    std::string name( void ) const { return _statname; }
    std::string resName( void ) const { return "NFSStat"; }
    void checkResources(const ResDB &rdb);
    void starttimer(void) { return _timer.start(); };
    void stoptimer(void) { return _timer.stop(); };
    double usecs(void) { return _timer.report_usecs(); };

protected:
    std::string _statname;
    std::string _statfile;

private:
    Timer _timer;
};


class NFSStats : public NFSMeter {
public:
    NFSStats(void);
    ~NFSStats(void);

    void checkevent( void );

    void checkResources(const ResDB &rdb);

private:
    unsigned long _lastcalls, _lastretrns, _lastauthrefresh;
};


class NFSDStats : public NFSMeter {
public:
    NFSDStats(void);
    ~NFSDStats(void);

    void checkevent( void );

    void checkResources(const ResDB &rdb);

protected:
    float _maxpackets;

private:
    unsigned long _lastTcp, _lastUdp, _lastNetCnt, _lastBad;
};


#endif
