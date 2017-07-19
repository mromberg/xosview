//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "memmeter.h"

#include <fstream>
#include <sstream>
#include <iomanip>


static const char * const MEMFILENAME = "/proc/meminfo";


MemMeter::MemMeter( void )
    : FieldMeterGraph( 3, "MEM", "USED/CACHE/FREE" ) {
}


MemMeter::~MemMeter( void ){
}


void MemMeter::checkResources(const ResDB &rdb){
    FieldMeterGraph::checkResources(rdb);

    setfieldcolor( 0, rdb.getColor( "memUsedColor" ) );
    setfieldcolor( 1, rdb.getColor( "memCacheColor" ) );
    setfieldcolor( 2, rdb.getColor( "memFreeColor" ) );
}


void MemMeter::checkevent( void ){
    getmeminfo();

    const float TOMEG = 1.0/1024.0/1024.0;
    logDebug << std::setprecision(1) << std::fixed
             << "t " << total_ * TOMEG << " "
             << "used "    << fields_[0] * TOMEG << " "
             << "cache  "  << fields_[1] * TOMEG << " "
             << "free "    << fields_[2] * TOMEG
             << std::endl;
}

//---------------------------------------------------------------------
// from 3.4.0/minix/fs/procfs/root.c
//buf_printf("%u %lu %lu %lu %lu\n", vsi.vsi_pagesize, vsi.vsi_total,
// 	    vsi.vsi_free, vsi.vsi_largest, vsi.vsi_cached);
//
// It is not clear to me what vsi_largest means.
//---------------------------------------------------------------------

void MemMeter::getmeminfo( void ){
    std::ifstream ifs(MEMFILENAME);

    if (!ifs)
        logFatal << "Could not open: " << MEMFILENAME << std::endl;

    unsigned int psize;
    unsigned long total, free, largest, cached;

    ifs >> psize >> total >> free >> largest >> cached;

    if (!ifs)
        logFatal << "Could not parse: " << MEMFILENAME << std::endl;

    unsigned long used = total - cached - free;

    logDebug << "psize: " << psize << ", "
             << "total: " << total << ", "
             << "used: " << used << ", "
             << "cache: " << cached << ", "
             << "free: " << free << ", "
             << "sum: " << used + cached + free
             << std::endl;
    
    total_ = (float)total * (float)psize;
    fields_[0] = (float)used * (float)psize;
    fields_[1] = (float)cached * (float)psize;
    fields_[2] = (float)free * (float)psize;

    if (total_)
        FieldMeterDecay::setUsed(total_ - fields_[2], total_);
}
