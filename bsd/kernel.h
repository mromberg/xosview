#ifndef __kernel_h__
#define __kernel_h__

//
//  NetBSD port:
//  Copyright (c) 1995, 1996, 1997-2002, 2015 by Brian Grayson (bgrayson@netbsd.org)
//
//  This file was written by Brian Grayson for the NetBSD and xosview
//    projects.
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//
// $Id: kernel.h,v 1.22 2003/10/14 01:53:17 bgrayson Exp $
//

#define KERNEL_H_CVSID	"$Id: kernel.h,v 1.22 2003/10/14 01:53:17 bgrayson Exp $"

void
BSDInit();

void
SetKernelName(const char* const kernelName);

void
BSDCPUInit();

//#if defined(XOSVIEW_NETBSD) && (__NetBSD_Version__ >= 104260000)
#if 0
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
BSDDiskInit();

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
