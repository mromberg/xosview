//
//
//  This file may be distributed under terms of the GPL
//
#ifndef nfsmeter_h
#define nfsmeter_h

#include "fieldmetergraph.h"
#include "timer.h"

class Host;

class NFSMeter : public FieldMeterGraph {
public:
    NFSMeter(const std::string &name, int nfields,
      const std::string &files, const std::string &statfile);
    virtual ~NFSMeter(void);

    virtual std::string name(void) const override { return _statname; }
    virtual std::string resName(void) const override { return "NFSStat"; }
    virtual void checkResources(const ResDB &rdb) override;
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
    virtual ~NFSStats(void);

    virtual void checkevent(void) override;

    virtual void checkResources(const ResDB &rdb) override;

private:
    unsigned long _lastcalls, _lastretrns, _lastauthrefresh;
};


class NFSDStats : public NFSMeter {
public:
    NFSDStats(void);
    virtual ~NFSDStats(void);

    virtual void checkevent(void) override;

    virtual void checkResources(const ResDB &rdb) override;

protected:
    float _maxpackets;

private:
    unsigned long _lastTcp, _lastUdp, _lastNetCnt, _lastBad;
};


#endif
