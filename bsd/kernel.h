#ifndef __netbsd_h__
#define __netbsd_h__

//
//  NetBSD port:  
//  Copyright (c) 1995, 1996, 1997 by Brian Grayson (bgrayson@ece.utexas.edu)
//
//  This file was written by Brian Grayson for the NetBSD and xosview
//    projects.
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//
// $Id: kernel.h,v 1.8 1997/06/28 05:35:16 bgrayson Exp $
//

#include <stdio.h>
#include <fcntl.h>
#include <kvm.h>
#include <nlist.h>

#include <sys/socket.h>
//  net/if.h is not protected from multiple inclusions, and apparently
//  something changed recently such that it is included via
//  sys/socket.h???  bgrayson
//#include <net/if.h>

#define NETBSD_H_CVSID	"$Id: kernel.h,v 1.8 1997/06/28 05:35:16 bgrayson Exp $"
void
NetBSDInit();

void
SetKernelName(const char* const kernelName);

void
NetBSDCPUInit();

void
NetBSDGetCPUTimes(long* timesArray);

void
NetBSDNetInit();

void
NetBSDGetNetInOut (long long * inbytes, long long * outbytes);

int
NetBSDSwapInit();

#ifdef HAVE_SWAPCTL
void
NetBSDGetSwapCtlInfo(int* total, int* free);
#endif

int
NetBSDDiskInit();

void
NetBSDGetDiskXFerBytes (unsigned long long * bytes);
#endif
