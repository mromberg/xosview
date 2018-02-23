//
//  Copyright (c) 1994, 1995, 2006, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef netmeter_h
#define netmeter_h

#include "cnetmeter.h"
#include "perfcount.h"



class NetMeter : public ComNetMeter {
public:
    NetMeter(void);

protected:
    virtual std::pair<float, float> getRates(void) override;

private:
    float _maxBandwidth;  // in bytes/sec
    PerfQuery _query;
    size_t _inIndex;

    void add(const std::vector<std::string> &adapters,
      const std::vector<std::string> &plist);
};


#endif
