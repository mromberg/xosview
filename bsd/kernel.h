#ifndef kernel_h
#define kernel_h

//
//  NetBSD port:
//  Copyright (c) 1995, 1996, 1997-2002, 2015, 2016, 2018
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


#if defined(__i386__) || defined(__x86_64)
extern size_t BSDGetCPUTemperature(std::vector<float> &temps,
  std::vector<float> &tjmax);

inline size_t BSDGetCPUTemperature(std::vector<float> &temps) {
    std::vector<float> notUsed;
    return BSDGetCPUTemperature(temps, notUsed);
}

inline size_t BSDGetCPUTemperature(void) {
    std::vector<float> notUsed1, notUsed2;
    return BSDGetCPUTemperature(notUsed1, notUsed2);
}
#endif

extern void BSDGetSensor(const std::string &name, const std::string &valname,
  float &value, std::string &unit);

extern bool BSDHasBattery(void);

enum BatteryState {
    XOSVIEW_BATT_NONE        = 0,
    XOSVIEW_BATT_CHARGING    = (1u << 0),
    XOSVIEW_BATT_DISCHARGING = (1u << 1),
    XOSVIEW_BATT_FULL        = (1u << 2),
    XOSVIEW_BATT_LOW         = (1u << 3),
    XOSVIEW_BATT_CRITICAL    = (1u << 4)
};

extern void BSDGetBatteryInfo(int &remaining, unsigned int &state);


#endif
