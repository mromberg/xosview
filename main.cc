//  
//  Copyright (c) 1994, 1995 by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: main.cc,v 1.2 1996/08/14 06:19:59 mromberg Exp $
//
#include "xosview.h"

main( int argc, char *argv[] ) {
  XOSView xosview( argc, argv );

  xosview.run();
}
