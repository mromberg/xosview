//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "perfcount.h"
#include "log.h"

#include <pdhmsg.h>


PerfCounter::PerfCounter(const std::string &path)
    : _path(path), _counter(0) {
}


bool PerfCounter::fmtValue(DWORD flags) {
    DWORD ret;
    PDH_STATUS status = PdhGetFormattedCounterValue(_counter,
      flags | PDH_FMT_NOCAP100, &ret, &_value);
    if (status != ERROR_SUCCESS) {
        logProblem << "PdhGetFormattedCounterValue() failed." << std::endl;
        return false;
    }

    return true;
}


double PerfCounter::doubleVal(void) {
    if (!fmtValue(PDH_FMT_DOUBLE))
        return 0.0;

    return _value.doubleValue;
}


long PerfCounter::longVal(void) {
    if (!fmtValue(PDH_FMT_LONG))
        return 0.0;

    return _value.longValue;
}


int64_t PerfCounter::llongVal(void) {
    if (!fmtValue(PDH_FMT_LARGE))
        return 0.0;

    return _value.largeValue;
}


PerfQuery::PerfQuery(void)
  : _query(0) {

    PDH_STATUS status = PdhOpenQuery(NULL, 0, &_query);
    if(status != ERROR_SUCCESS) {
        logProblem << "PdhOpenQuery() failed" << std::endl;
        _query = 0;
    }
}


PerfQuery::~PerfQuery(void) {
    if (_query) {
        if (PdhCloseQuery(_query) != ERROR_SUCCESS)
            logProblem << "PdhCloseQuery() failed" << std::endl;
    }
}


bool PerfQuery::add(const std::string &path) {
    PerfCounter pc(path);
    if (PdhAddCounter(_query, TEXT(pc.path().c_str()), 0, &pc.counter())
      != ERROR_SUCCESS)
        return false;

    _counters.push_back(pc);
    return true;
}


bool PerfQuery::query(void) const {
    PDH_STATUS status = PdhCollectQueryData(_query);
    if(status != ERROR_SUCCESS)
        return false;

    return true;
}


std::vector<std::string> PerfQuery::expand(const std::string &path) {
    std::vector<std::string> rval;

    DWORD plistLen = 0;
    PDH_STATUS status = PdhExpandWildCardPath(NULL, path.c_str(),
      NULL, &plistLen, 0);
    if (status == (PDH_STATUS)PDH_MORE_DATA) {
        std::vector<TCHAR> plist(plistLen);
        status = PdhExpandWildCardPath(NULL, path.c_str(),
          plist.data(), &plistLen, 0);

        if (status == ERROR_SUCCESS) {
            LPSTR lp = plist.data();
            while (*lp) {
                std::string ep(lp);
                rval.push_back(ep);
                lp += ep.size() + 1;
            }
        }
        else
            logProblem << "PdhExpandWildCardPath() failed" << std::endl;

    }
    else
        logProblem << "PdhExpandWildCardPath() failed" << std::endl;

    return rval;
}


std::map<std::string, std::string> PerfQuery::parse(
    const std::string &path) {

    std::map<std::string, std::string> rval;

    DWORD pSize = 0;
    PDH_STATUS status = PdhParseCounterPath(path.c_str(), NULL, &pSize, 0);

    if (status == (PDH_STATUS)PDH_MORE_DATA) {
        std::vector<BYTE> pbuf(pSize);
        status = PdhParseCounterPath(path.c_str(),
          (PDH_COUNTER_PATH_ELEMENTS *)pbuf.data(), &pSize, 0);

        if (status == ERROR_SUCCESS) {
            PDH_COUNTER_PATH_ELEMENTS *pe =
                (PDH_COUNTER_PATH_ELEMENTS *)pbuf.data();
            if (pe->szMachineName)
                rval["mname"] = pe->szMachineName;
            if (pe->szObjectName)
                rval["oname"] = pe->szObjectName;
            if (pe->szInstanceName)
                rval["iname"] = pe->szInstanceName;
            if (pe->szParentInstance)
                rval["parent"] = pe->szParentInstance;
            if (pe->szCounterName)
                rval["cname"] = pe->szCounterName;
        }
        else
            logProblem << "PdhParseCounterPath() failed." << std::endl;
    }
    else
        logProblem << "PdhParseCounterPath() failed." << std::endl;

    return rval;
}
