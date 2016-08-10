#ifndef XOSVKERNEL_H
#define XOSVKERNEL_H

//
//  NetBSD port:
//  Copyright (c) 1995, 1996, 1997-2002, 2015, 2016
//  by Brian Grayson (bgrayson@netbsd.org)
//
//  This file was written by Brian Grayson for the NetBSD and xosview
//    projects.
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//

#include <string>
#include <vector>


void BSDInit();

void SetKernelName(const std::string &kernelName);

void BSDPageInit();

void BSDGetMemPageStats(std::vector<uint64_t> &meminfo,
  std::vector<uint64_t> &pageinfo);

inline void BSDGetMemStats(std::vector<uint64_t> &meminfo) {
    std::vector<uint64_t> pinfo;
    BSDGetMemPageStats(meminfo, pinfo);
}

inline void BSDGetPageStats(std::vector<uint64_t> &pageinfo) {
    std::vector<uint64_t> minfo;
    BSDGetMemPageStats(minfo, pageinfo);
}

void BSDCPUInit();

void BSDGetCPUTimes(std::vector<uint64_t> &timesArray, unsigned int cpu = 0);

bool BSDSwapInit();

void BSDGetSwapInfo(uint64_t &total, uint64_t &free);

bool BSDDiskInit();

uint64_t BSDGetDiskXFerBytes(uint64_t &read_bytes, uint64_t &write_bytes);

int BSDNumInts();

void BSDGetIntrStats(std::vector<uint64_t> &intrCount,
  std::vector<unsigned int> &intrNbrs);

inline void BSDGetIntrCount(std::vector<uint64_t> &intrCount) {
    std::vector<unsigned int> notUsed;
    BSDGetIntrStats(intrCount, notUsed);
}

#if defined(__i386__) || defined(__x86_64)
unsigned int BSDGetCPUTemperature(std::vector<float> &temps,
  std::vector<float> &tjmax);

inline unsigned int BSDGetCPUTemperature(std::vector<float> &temps) {
    std::vector<float> notUsed;
    return BSDGetCPUTemperature(temps, notUsed);
}

inline unsigned int BSDGetCPUTemperature(void) {
    std::vector<float> notUsed1, notUsed2;
    return BSDGetCPUTemperature(notUsed1, notUsed2);
}
#endif

void BSDGetSensor(const std::string &name, const std::string &valname,
  float &value, std::string &unit);

bool BSDHasBattery();

enum BatteryState {
    XOSVIEW_BATT_NONE        = 0,
    XOSVIEW_BATT_CHARGING    = (1u << 0),
    XOSVIEW_BATT_DISCHARGING = (1u << 1),
    XOSVIEW_BATT_FULL        = (1u << 2),
    XOSVIEW_BATT_LOW         = (1u << 3),
    XOSVIEW_BATT_CRITICAL    = (1u << 4)
};

void BSDGetBatteryInfo(int &remaining, unsigned int &state);


#endif
