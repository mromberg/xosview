#ifndef __netbsd_h__
#define __netbsd_h__

//
//  NetBSD port:  
//  Copyright (c) 1995, 1996, 1997 by Brian Grayson (bgrayson@ece.utexas.edu)
//
//  This file was written by Brian Grayson for the NetBSD and xosview
//    projects.
//  This file may be distributed under terms of the GPL or of the BSD
//    copyright, whichever you choose.  The full copyright notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//
// $Id: netbsd.h,v 1.6 1997/02/14 06:01:57 bgrayson Exp $
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

#define NETBSD_H_CVSID	"$Id: netbsd.h,v 1.6 1997/02/14 06:01:57 bgrayson Exp $"
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

int
NetBSDDiskInit();

void
NetBSDGetDiskXFerBytes (unsigned long long * bytes);
#endif
