#ifndef __kernel_h__
#define __kernel_h__

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
//

#include "config.h"

// To mark code that won't compile
#define DEAD_CODE 0

void
BSDInit();

void
SetKernelName(const char* const kernelName);

void
BSDPageInit();

#if defined(UVM)
#if DEAD_CODE
void
BSDGetUVMPageStats(struct uvmexp* uvmp);
#endif // DEAD_CODE
#else
void
BSDGetPageStats(struct vmmeter* vmp);
#endif

void
BSDCPUInit();

#if defined(XOSVIEW_NETBSD) && (__NetBSD_Version__ >= 104260000)
void
BSDGetCPUTimes(u_int64_t* timesArray);
#else
void
BSDGetCPUTimes(long* timesArray);
#endif

int
BSDNetInit();

void
BSDGetNetInOut (long long * inbytes, long long * outbytes);

int
BSDSwapInit();

#ifdef HAVE_SWAPCTL
void
BSDGetSwapCtlInfo(unsigned long long* total, unsigned long long* free);
#endif

int
BSDDiskInit();

#if DEAD_CODE
void
BSDGetDiskXFerBytes (unsigned long long * bytes);
#endif

#ifdef XOSVIEW_FREEBSD
void
FreeBSDGetBufspace(int* bfsp);
#endif


#define NUM_INTR	16

int
BSDIntrInit();

int
BSDNumInts();

void
BSDGetIntrStats (unsigned long intrCount[NUM_INTR]);

#endif
