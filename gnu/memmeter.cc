//
//  Copyright (c) 1994, 1995, 2015, 2016, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//  2007 by Samuel Thibault ( samuel.thibault@ens-lyon.org )
//
//  This file may be distributed under terms of the GPL
//
#include "memmeter.h"

#include <fstream>

extern "C" {
#include <mach/vm_statistics.h>
#include <mach/mach_traps.h>
#include <mach/mach_interface.h>
}


MemMeter::MemMeter(void)
    : FieldMeterGraph(4, "MEM", "ACT/INACT/WIRE/FREE") {
}


void MemMeter::checkResources(const ResDB &rdb) {
    FieldMeterGraph::checkResources(rdb);

    setfieldcolor(0, rdb.getColor( "memActiveColor"));
    setfieldcolor(1, rdb.getColor( "memInactiveColor"));
    setfieldcolor(2, rdb.getColor( "memCacheColor"));
    setfieldcolor(3, rdb.getColor( "memFreeColor"));
}


void MemMeter::checkevent(void) {

    struct vm_statistics vmstats;
    const auto err = vm_statistics (mach_task_self(), &vmstats);
    if (err)
        logFatal << "vm_statistics(): " << util::strerror(err) << std::endl;

    _fields[0] = vmstats.active_count;
    _fields[1] = vmstats.inactive_count;
    _fields[2] = vmstats.wire_count;
    _fields[3] = vmstats.free_count;
    _total = _fields[0] + _fields[1] + _fields[2] + _fields[3];

    const float used = static_cast<double>(_total - _fields[3])
        * vmstats.pagesize;
    const float total = static_cast<double>(_total) * vmstats.pagesize;

    FieldMeterDecay::setUsed(used, total);
}
