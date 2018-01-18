//
//  Copyright (c) 2015, 2017, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
//  Autoheader/autoconf generated config.h does not
// have include guards.  Rather than pondering the mysteries of
// why, I'm just gonna put them here.  No one should directly include
// config.h
#ifndef configxosv_h
#define configxosv_h

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


//------------------------------------------------------------------
// GCC7 warns about switch/case fallthough.   Avoiding the warning
// is a real PITA as it requires a newish c++0x compiler.  This
// macro attempts to test for the compilers support of the
// fallthough attribute.
//------------------------------------------------------------------
#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(fallthrough)
#define FALLTHROUGH [[fallthrough]]
#else
#define FALLTHROUGH ;
#endif
#else
#define FALLTHROUGH ;
#endif


#endif
