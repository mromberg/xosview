//  
//  Copyright (c) 1994, 1995 by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: main.cc,v 1.3 1996/11/19 03:59:26 bgrayson Exp $
//
#include "general.h"
#include "xosview.h"

CVSID("$Id: main.cc,v 1.3 1996/11/19 03:59:26 bgrayson Exp $");

main( int argc, char *argv[] ) {
  XOSView xosview( argc, argv );

  xosview.run();
}
