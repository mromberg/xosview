#ifndef __netbsd_h__
#define __netbsd_h__

//
//  NetBSD port:  
//  Copyright (c) 1995 Brian Grayson(bgrayson@pine.ece.utexas.edu)
//
//
// $Id: kernel.h,v 1.2 1996/08/14 06:21:41 mromberg Exp $
//

#include <stdio.h>
#include <fcntl.h>
#include <kvm.h>
#include <nlist.h>

#include <sys/socket.h>
#include <net/if.h>

void
NetBSDCPUInit();

void
NetBSDGetCPUTimes(long* timesArray);

void
NetBSDNetInit();

void
NetBSDGetNetInOut (long long * inbytes, long long * outbytes);

#endif
