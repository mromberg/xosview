//
//  Copyright (c) 1994, 1995, 2015, 2016 by Mike Romberg ( mike-romberg@comcast.net )
//  2007 by Samuel Thibault ( samuel.thibault@ens-lyon.org )
//
//  This file may be distributed under terms of the GPL
//
#include "memmeter.h"

#include <fstream>
#include <sstream>

extern "C" {
#include <mach/mach_traps.h>
#include <mach/mach_interface.h>
}

MemMeter::MemMeter( void )
    : FieldMeterGraph( 4, "MEM", "ACT/INACT/WIRE/FREE" ){
}

MemMeter::~MemMeter( void ){
}

void MemMeter::checkResources(const ResDB &rdb){
    FieldMeterGraph::checkResources(rdb);

    setfieldcolor( 0, rdb.getColor( "memActiveColor" ) );
    setfieldcolor( 1, rdb.getColor( "memInactiveColor" ) );
    setfieldcolor( 2, rdb.getColor( "memCacheColor" ) );
    setfieldcolor( 3, rdb.getColor( "memFreeColor" ) );
}

void MemMeter::checkevent( void ){
    kern_return_t err;

    err = vm_statistics (mach_task_self(), &vmstats);
    if (err)
        logFatal << "vm_statistics(): " << util::strerror(err) << std::endl;

    fields_[0] = vmstats.active_count;
    fields_[1] = vmstats.inactive_count;
    fields_[2] = vmstats.wire_count;
    fields_[3] = vmstats.free_count;
    total_ = fields_[0] + fields_[1] + fields_[2] + fields_[3];

    float used = static_cast<double>(total_ - fields_[3]) * vmstats.pagesize;
    float total = static_cast<double>(total_) * vmstats.pagesize;

    FieldMeterDecay::setUsed(used, total);
}
