//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "cswapmeter.h"

#ifdef USESYSCALLS
#if defined(GNULIBC) || defined(__GLIBC__)
#include <sys/sysinfo.h>
#else
#include <syscall.h>
#include <linux/kernel.h>
#endif
#endif


static const char * const MEMFILENAME = "/proc/meminfo";


ComSwapMeter::ComSwapMeter( XOSView *parent )
: FieldMeterGraph( parent, 2, "SWAP", "USED/FREE" ){
}


ComSwapMeter::~ComSwapMeter( void ){
}


void ComSwapMeter::checkResources(const ResDB &rdb){
    FieldMeterGraph::checkResources(rdb);

    setfieldcolor( 0, rdb.getColor( "swapUsedColor" ) );
    setfieldcolor( 1, rdb.getColor( "swapFreeColor" ) );
    priority_ = util::stoi (rdb.getResource( "swapPriority" ));
    dodecay_ = rdb.isResourceTrue( "swapDecay" );
    useGraph_ = rdb.isResourceTrue( "swapGraph" );
    setUsedFormat (rdb.getResource("swapUsedFormat"));
    decayUsed(rdb.isResourceTrue("swapUsedDecay"));
}


void ComSwapMeter::checkevent( void ){
    std::pair<uint64_t, uint64_t> swinfo = getswapinfo();
    total_ = swinfo.first;
    fields_[0] = total_ - swinfo.second;
    fields_[1] = swinfo.second;

    if (swinfo.first)
        setUsed (fields_[0], total_);
}


#ifdef USESYSCALLS
std::pair<uint64_t, uint64_t> ComSwapMeter::getswapinfo( void ){
    struct sysinfo sinfo;
    int unit;

#if defined(GNULIBC) || defined(__GLIBC__)
    sysinfo(&sinfo);
#else
    syscall( SYS_sysinfo, &sinfo );
#endif

    unit = (sinfo.mem_unit ? sinfo.mem_unit : 1);

    return std::make_pair(sinfo.totalswap * unit, sinfo.freeswap * unit);
}
#endif
