//  Copyright (c) 1995, 1996, 1997-2002 by Brian Grayson (bgrayson@netbsd.org)
//  This code is borrowed HEAVILY from the systat source code in the
//  NetBSD distribution.
//  
//
// $Id: swapinternal.h,v 1.6 2002/03/22 03:23:41 bgrayson Exp $
//

//  Header file for the swap internal/NetBSD-specific code.

int
BSDInitSwapInfo();

void
BSDGetSwapInfo(int* total, int* free);

void
BSDGetSwapCtlInfo(int* total, int* free);
