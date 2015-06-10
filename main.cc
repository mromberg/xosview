//
//  Copyright (c) 1994, 1995, 2006, 2007, 2015 by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: main.cc,v 1.8 2007/09/12 22:11:52 romberg Exp $
//
#include "general.h"
#include "xosview.h"

CVSID("$Id: main.cc,v 1.8 2007/09/12 22:11:52 romberg Exp $");

int main( int argc, char *argv[] ) {
  /*  Icky.  Need to check for -name option here.  */
  char** argp = argv;
  const char* instanceName = "xosview";	// Default value.
  while (argp && *argp)
  {
    if (!strncmp(*argp, "-name", 6))
      instanceName = argp[1];
    argp++;
  }  //  instanceName will end up pointing to the last such -name option.
  XOSView xosview( instanceName, argc, argv );

  xosview.run();

  return 0;
}
