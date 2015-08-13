#ifndef XOSVKERNEL_H
#define XOSVKERNEL_H

//
//  NetBSD port:
//  Copyright (c) 1995, 1996, 1997-2002, 2015
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

#include "defines.h"
#include <string>

void
BSDInit();

void
SetKernelName(const std::string &kernelName);

int
BSDGetCPUSpeed();

void
BSDPageInit();

void
BSDGetPageStats(uint64_t *meminfo, uint64_t *pageinfo);

void
BSDCPUInit();

void
BSDGetCPUTimes(uint64_t *timesArray, unsigned int cpu = 0);

int
BSDNetInit();

void
BSDGetNetInOut(uint64_t *inbytes, uint64_t *outbytes,
  const std::string &netIface, bool ignored);

int
BSDSwapInit();

void
BSDGetSwapInfo(uint64_t *total, uint64_t *free);

int
BSDDiskInit();

uint64_t
BSDGetDiskXFerBytes(uint64_t *read_bytes, uint64_t *write_bytes);

int
BSDIntrInit();

int
BSDNumInts();

void
BSDGetIntrStats(uint64_t *intrCount, unsigned int *intrNbrs);

int
BSDCountCpus(void);

#if defined(__i386__) || defined(__x86_64)
unsigned int
BSDGetCPUTemperature(float *temps, float *tjmax);
#endif

void
BSDGetSensor(const std::string &name, const char *valname, float *value,
  std::string &unit);

bool
BSDHasBattery();

void
BSDGetBatteryInfo(int *remaining, unsigned int *state);


#endif
