//  Copyright (c) 1995, 1996, 1997 by Brian Grayson (bgrayson@ece.utexas.edu)
//  This code is borrowed HEAVILY from the systat source code in the
//  NetBSD distribution.
//  
//
// $Id: swapinternal.h,v 1.5 1998/02/12 05:04:07 bgrayson Exp $
//

//  Header file for the swap internal/NetBSD-specific code.

int
BSDInitSwapInfo();

void
BSDGetSwapInfo(int* total, int* free);

void
BSDGetSwapCtlInfo(int* total, int* free);
