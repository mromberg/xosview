//  Copyright (c) 1995 by Brian Grayson (bgrayson@pine.ece.utexas.edu)
//  This code is borrowed HEAVILY from the systat source code in the
//  NetBSD distribution.
//  
//
// $Id: swapinternal.h,v 1.2 1996/08/14 06:21:49 mromberg Exp $
//

//  Header file for the swap internal/NetBSD-specific code.

int
NetBSDInitSwapInfo();

void
NetBSDGetSwapInfo(int* total, int* free);
