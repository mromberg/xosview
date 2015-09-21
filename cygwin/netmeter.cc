//
//  Copyright (c) 1994, 1995, 2002, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "netmeter.h"
#include "winders.h"

#include <iomanip>
#include <cctype>





NetMeter::NetMeter( XOSView *parent)
    : ComNetMeter(parent), _inIndex(0) {

    std::vector<std::string> adapters = WinHardware::getAdapters();
    logDebug << "network adapters: " << adapters << std::endl;

    add(adapters,
      PerfQuery::expand("\\Network Interface(*)\\Bytes Received/sec"));

    _inIndex = _query.counters().size();

    add(adapters,
      PerfQuery::expand("\\Network Interface(*)\\Bytes Sent/sec"));

    _query.query();
}


std::pair<float, float> NetMeter::getRates(void) {
    _query.query();

    std::pair<float, float> rval(0, 0);
    for (size_t i = 0 ; i < _inIndex ; i++)
        rval.first += _query.counters()[i].doubleVal();
    for (size_t i = _inIndex ; i < _query.counters().size() ; i++)
        rval.second += _query.counters()[i].doubleVal();

    return rval;
}


struct AlphaEqual {
    bool operator()(const TCHAR &a, const TCHAR &b) {
        if (!std::isalpha(a) && !std::isalpha(b))
            return true;
        if (a == b)
            return true;
        return false;
    }
};


void NetMeter::add(const std::vector<std::string> &adapters,
  const std::vector<std::string> &plist) {

    for (size_t i = 0 ; i < plist.size() ; i++) {
        std::string iname = PerfQuery::parse(plist[i])["iname"];
        for (size_t j = 0 ; j < adapters.size() ; j++)
            if (std::equal(iname.begin(), iname.end(), adapters[j].begin(),
                AlphaEqual())) {
                if (!_query.add(plist[i])) {
                    logProblem << "failed to add: " << plist[i] << std::endl;
                }
                else {
                    logDebug << "Add: " << plist[i] << std::endl;
                }
                break;
            }
    }
}
