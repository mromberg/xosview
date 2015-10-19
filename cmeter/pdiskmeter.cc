//
//  Copyright (c) 2015
//  by Mike Romberg (mike-romberg@comcast.net)
//
//  This file may be distributed under terms of the GPL
//
#include "pdiskmeter.h"
#include "fsutil.h"

#include <limits>


PrcDiskMeter::PrcDiskMeter(void)
    : ComDiskMeter(), _last(getTotals()) {
}

PrcDiskMeter::~PrcDiskMeter(void) {
}


std::pair<double, double> PrcDiskMeter::getRate(void) {
    _timer.stop();
    double etime = _timer.report_usecs() / 1000000.0;

    std::pair<uint64_t, uint64_t> totals(getTotals());

    std::pair<double, double> rval((totals.first - _last.first) / etime,
      (totals.second - _last.second) / etime);

    _last = totals;

    return rval;
}


std::pair<uint64_t, uint64_t> PrcDiskMeter::getTotals(void) {
    static bool first = true;
    static bool useSys = false;
    if (first) {
        first = false;
        if (util::fs::isdir("/sys/block"))
            useSys = true;
        else if (!util::fs::isfile("/proc/diskstats"))
            logFatal << "failed to find /sys/block or /proc/diskstats.\n";
    }

    std::pair<uint64_t, uint64_t> totals = useSys ? getSysTotals()
        : getPrcTotals();

    _timer.start();

    return totals;
}


inline static void skip(std::ifstream &ifs, size_t count) {
    for (size_t i = 0 ; i < count ; i++) {
        if (!ifs)
            break;
        ifs.ignore(std::numeric_limits<std::streamsize>::max(), ' ');
    }
}

std::pair<uint64_t, uint64_t> PrcDiskMeter::getSysTotals(void) const {
    // just sum up everything in /sys/block/*/stat
    std::pair<uint64_t, uint64_t> rval(0, 0);
    const std::string dname("/sys/block/");

    std::vector<std::string> devs = util::fs::listdir(dname);
    for (size_t i = 0 ; i < devs.size() ; i++) {
        std::string statFile(dname + devs[i] + "/stat");
        std::ifstream ifs(statFile.c_str());
        if (ifs) {
            // 3rd field is read sectors and 7th is write
            // secotor is UNIX 512 bytes and not device or fs block
            uint64_t read = 0, write = 0;
            skip(ifs, 2);
            ifs >> read;
            skip(ifs, 3);
            ifs >> write;
            if (ifs) {
                rval.first += read * 512;
                rval.second += write * 512;
            }
        }
    }

    return rval;
}


std::pair<uint64_t, uint64_t> PrcDiskMeter::getPrcTotals(void) const {
    std::pair<uint64_t, uint64_t> rval(0, 0);

    std::ifstream ifs("/proc/diskstats");

    int major, minor;
    uint64_t read, write;
    while (ifs) {
        ifs >> major >> minor;
        if (ifs && minor == 0) {
            skip(ifs, 3);
            ifs >> read;
            skip(ifs, 3);
            ifs >> write;
            ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (ifs) {
                rval.first += read * 512;
                rval.second += write * 512;
            }
        }
        else
            ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    return rval;
}
