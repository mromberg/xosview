//
//  Copyright (c) 2015, 2017
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
//  Autoheader/autoconf generated config.h does not
// have include guards.  Rather than pondering the mysteries of
// why, I'm just gonna put them here.  No one should directly include
// config.h
#ifndef CONFIGXOSV_H
#define CONFIGXOSV_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


//------------------------------------------------------------------
// GCC7 warns about switch/case fallthough.   Avoiding the warning
// is a real PITA as it requires a newish c++0x compiler.  This
// macro attempts to test for the compilers support of the
// fallthough attribute.
//------------------------------------------------------------------
#define FALLTHROUGH ;
#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(fallthrough)
#define FALLTHROUGH [[fallthrough]]
#endif
#endif


#endif
