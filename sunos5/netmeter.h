//
//  Copyright (c) 1994, 1995, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#ifndef netmeter_h
#define netmeter_h

#include "cnetmeter.h"
#include "kstats.h"



class NetMeter : public ComNetMeter {
public:
    NetMeter(kstat_ctl_t *kc);
    virtual ~NetMeter(void);

    virtual void checkResources(const ResDB &rdb) override;

protected:
    virtual std::pair<float, float> getRates(void) override;

private:
    uint64_t _lastBytesIn, _lastBytesOut;
    kstat_ctl_t *_kc;
    KStatList *_nets;
    std::string _netIface;
    bool _ignored;
    int _socket;
};


#endif
