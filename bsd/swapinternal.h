//  Copyright (c) 1995, 1996, 1997 by Brian Grayson (bgrayson@ece.utexas.edu)
//  This code is borrowed HEAVILY from the systat source code in the
//  NetBSD distribution.
//  
//
// $Id: swapinternal.h,v 1.3 1997/02/14 06:02:10 bgrayson Exp $
//

//  Header file for the swap internal/NetBSD-specific code.

int
NetBSDInitSwapInfo();

void
NetBSDGetSwapInfo(int* total, int* free);
