//
//  Copyright (c) 1994, 1995, 2002, 2006, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "netmeter.h"
#include "winders.h"

#include <iomanip>
#include <cctype>



NetMeter::NetMeter(void)
    : ComNetMeter(), _inIndex(0) {

    const auto adapters = WinHardware::getAdapters();
    logDebug << "network adapters: " << adapters << std::endl;

    add(adapters, PerfQuery::expand(
        "\\Network Interface(*)\\Bytes Received/sec"));

    _inIndex = _query.counters().size();

    add(adapters, PerfQuery::expand("\\Network Interface(*)\\Bytes Sent/sec"));

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


void NetMeter::add(const std::vector<std::string> &adapters,
  const std::vector<std::string> &plist) {

    for (const auto &p : plist) {
        const std::string iname = PerfQuery::parse(p)["iname"];
        for (const auto &adapter : adapters)
            if (std::equal(iname.begin(), iname.end(), adapter.begin(),
                [](TCHAR a, TCHAR b){ return (!std::isalpha(a)
                      && !std::isalpha(b)) ? true : a == b; })) {
                if (!_query.add(p)) {
                    logProblem << "failed to add: " << p << std::endl;
                }
                else {
                    logDebug << "Add: " << p << std::endl;
                }
                break;
            }
    }
}
