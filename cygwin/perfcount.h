//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef perfcount_h
#define perfcount_h


#include <string>
#include <vector>
#include <map>
#include <cstdint>

#include <pdh.h>




class PerfCounter {
public:
    PerfCounter(const std::string &path);

    const std::string &path(void) const { return _path; }
    const HCOUNTER &counter(void) const { return _counter; }
    HCOUNTER &counter(void) { return _counter; }

    double doubleVal(void);
    long longVal(void);
    int64_t llongVal(void);

private:
    std::string _path;
    HCOUNTER _counter;
    PDH_FMT_COUNTERVALUE _value;

    bool fmtValue(DWORD flags);
};


class PerfQuery {
public:
    PerfQuery(void);
    ~PerfQuery(void);

    bool add(const std::string &path);
    bool query(void) const;

    std::vector<PerfCounter> &counters(void) { return _counters; }
    const std::vector<PerfCounter> &counters(void) const { return _counters; }

    static std::vector<std::string> expand(const std::string &path);
    static std::map<std::string, std::string> parse(const std::string &path);

private:
    HQUERY _query;
    std::vector<PerfCounter> _counters;
};


#endif
