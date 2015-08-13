//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "swapmeter.h"
#include "xosview.h"

#include <fstream>
#include <sstream>

#include <stdlib.h>

#ifdef USESYSCALLS
#if defined(GNULIBC) || defined(__GLIBC__)
#include <sys/sysinfo.h>
#else
#include <syscall.h>
#include <linux/kernel.h>
#endif
#endif


static const char * const MEMFILENAME = "/proc/meminfo";


SwapMeter::SwapMeter( XOSView *parent )
: FieldMeterGraph( parent, 2, "SWAP", "USED/FREE" ){
}


SwapMeter::~SwapMeter( void ){
}


void SwapMeter::checkResources(const ResDB &rdb){
    FieldMeterGraph::checkResources(rdb);

    setfieldcolor( 0, rdb.getColor( "swapUsedColor" ) );
    setfieldcolor( 1, rdb.getColor( "swapFreeColor" ) );
    priority_ = util::stoi (rdb.getResource( "swapPriority" ));
    dodecay_ = rdb.isResourceTrue( "swapDecay" );
    useGraph_ = rdb.isResourceTrue( "swapGraph" );
    setUsedFormat (rdb.getResource("swapUsedFormat"));
    decayUsed(rdb.isResourceTrue("swapUsedDecay"));
}


void SwapMeter::checkevent( void ){
    getswapinfo();
    drawfields(parent_->g());
}


#ifdef USESYSCALLS
void SwapMeter::getswapinfo( void ){
    struct sysinfo sinfo;
    int unit;

#if defined(GNULIBC) || defined(__GLIBC__)
    sysinfo(&sinfo);
#else
    syscall( SYS_sysinfo, &sinfo );
#endif

    unit = (sinfo.mem_unit ? sinfo.mem_unit : 1);

    total_ = static_cast<float>(sinfo.totalswap) * unit;
    fields_[0] = static_cast<float>(sinfo.totalswap - sinfo.freeswap) * unit;
    fields_[1] = static_cast<float>(sinfo.freeswap) * unit;

    if ( total_ == 0 ){
        total_ = 1;
        fields_[0] = 0;
        fields_[1] = 1;
    }

    if (total_)
        setUsed (fields_[0], total_);
}
#else
void SwapMeter::getswapinfo( void ){
    std::ifstream meminfo( MEMFILENAME );
    if ( !meminfo ){
        logFatal << "Cannot open file : " << MEMFILENAME << std::endl;
    }

    total_ = fields_[0] = fields_[1] = 0;

    std::string ignore;

    // Get the info from the "standard" meminfo file.
    while (!meminfo.eof()){
        std::string buf;
        std::getline(meminfo, buf);
        std::istringstream line(buf);

        if (buf.substr(0, 9) == "SwapTotal")
            line >> ignore >> total_;

        if (buf.substr(0, 8) == "SwapFree")
            line >> ignore >> fields_[1];
    }

    fields_[0] = total_ - fields_[1];

    if ( total_ == 0 ){
        total_ = 1;
        fields_[0] = 0;
        fields_[1] = 1;
    }

    if (total_)
        setUsed (fields_[0], total_);
}
#endif
