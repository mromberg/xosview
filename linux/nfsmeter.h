//
//
//  This file may be distributed under terms of the GPL
//
#ifndef nfsmeter_h
#define nfsmeter_h

#include "fieldmetergraph.h"



class NFSMeter : public FieldMeterGraph {
public:
    NFSMeter(const std::string &name, size_t nfields,
      const std::string &files, const std::string &statfile);

    virtual std::string name(void) const override { return _statname; }
    virtual std::string resName(void) const override { return "NFSStat"; }
    virtual void checkResources(const ResDB &rdb) override;

protected:
    std::string _statname;
    std::string _statfile;
};



class NFSStats : public NFSMeter {
public:
    NFSStats(void);

    virtual void checkevent(void) override;
    virtual void checkResources(const ResDB &rdb) override;

private:
    unsigned long _lastcalls, _lastretrns, _lastauthrefresh;
};



class NFSDStats : public NFSMeter {
public:
    NFSDStats(void);

    virtual void checkevent(void) override;
    virtual void checkResources(const ResDB &rdb) override;

private:
    float _maxpackets;
    unsigned long _lastTcp, _lastUdp, _lastNetCnt, _lastBad;
};


#endif
