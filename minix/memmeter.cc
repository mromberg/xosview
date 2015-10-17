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


MemMeter::MemMeter( XOSView *parent ) : FieldMeterGraph( parent, 4, "MEM",
  "USED/CACHE/CFREE/FREE" ){
}


MemMeter::~MemMeter( void ){
}


void MemMeter::checkResources(const ResDB &rdb){
    FieldMeterGraph::checkResources(rdb);

    setfieldcolor( 0, rdb.getColor( "memUsedColor" ) );
    setfieldcolor( 1, rdb.getColor( "memCacheColor" ) );
    setfieldcolor( 2, rdb.getColor( "memInactiveColor" ) );
    setfieldcolor( 3, rdb.getColor( "memFreeColor" ) );
}


void MemMeter::checkevent( void ){
    getmeminfo();

    const float TOMEG = 1.0/1024.0/1024.0;
    logDebug << std::setprecision(1) << std::fixed
             << "t " << total_ * TOMEG << " "
             << "used "    << fields_[0] * TOMEG << " "
             << "cache "   << fields_[1] * TOMEG << " "
             << "contig "  << fields_[2] * TOMEG << " "
             << "free "    << fields_[3] * TOMEG
             << std::endl;
}


void MemMeter::getmeminfo( void ){
    std::ifstream ifs(MEMFILENAME);

    if (!ifs)
        logFatal << "Could not open: " << MEMFILENAME << std::endl;

    unsigned long long psize, total, free, largest, cached;

    ifs >> psize >> total >> free >> largest >> cached;

    if (!ifs)
        logFatal << "Could not parse: " << MEMFILENAME << std::endl;

    logDebug << "psize: " << psize << ", "
             << "total: " << total << ", "
             << "free: " << free << ", "
             << "lrg: " << largest << ", "
             << "cache: " << cached << std::endl;

    total_ = (float)total * (float)psize;
    fields_[1] = (float)cached * (float)psize;
    fields_[2] = (float)largest * (float)psize;
    fields_[3] = (float)(free - largest) * (float)psize;
    // The used is whatever is left.  Assuming the largest is part
    // of this count.  So, subtract the largest from used
    fields_[0] = total_ - (fields_[1] + fields_[2] + fields_[3]);
    fields_[0] = fields_[0] >= 0 ? fields_[0] : 0;

    if (total_)
        FieldMeterDecay::setUsed(fields_[0], total_);
}
