#ifndef __netbsd_h__
#define __netbsd_h__

//
//  NetBSD port:  
//  Copyright (c) 1995 Brian Grayson(bgrayson@pine.ece.utexas.edu)
//
//
// $Id: netbsd.h,v 1.3 1996/11/19 06:26:45 bgrayson Exp $
//

#include <stdio.h>
#include <fcntl.h>
#include <kvm.h>
#include <nlist.h>

#include <sys/socket.h>
#include <net/if.h>

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
NetBSDDiskInit();

void
NetBSDGetDiskXFerBytes (unsigned long long * bytes);
#endif
