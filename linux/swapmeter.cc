//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "swapmeter.h"

#ifdef USESYSCALLS
#if defined(GNULIBC) || defined(__GLIBC__)
#include <sys/sysinfo.h>
#else
#include <syscall.h>
#include <linux/kernel.h>
#endif
#endif



std::pair<uint64_t, uint64_t> SwapMeter::getswapinfo( void ){

#ifdef USESYSCALLS
    struct sysinfo sinfo;
    int unit;

#if defined(GNULIBC) || defined(__GLIBC__)
    sysinfo(&sinfo);
#else
    syscall( SYS_sysinfo, &sinfo );
#endif

    unit = (sinfo.mem_unit ? sinfo.mem_unit : 1);

    std::pair<uint64_t, uint64_t> rval(sinfo.totalswap * unit,
      sinfo.freeswap * unit);

    return rval;
#else
    return PrcSwapMeter::getswapinfo();
#endif
}
