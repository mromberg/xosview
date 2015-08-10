//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "cpumeter.h"
#include "xosvproc.h"

#include <fstream>

#include <minix/type.h>
#include <minix/com.h>
#include <minix/procfs.h>


CPUMeter::CPUMeter(XOSView *parent, unsigned int cpu)
    : FieldMeterGraph( parent, 5, util::toupper(CPUMeter::cpuStr(cpu)),
      "USR/NI/KERN/SYS/IDL"), _cpu(cpu) {

    logDebug << "CPU: " << _cpu << std::endl;  // mostly to suppress warning
    _lastTicks = getTicks();
}


CPUMeter::~CPUMeter( void ){
}


void CPUMeter::checkResources(const ResDB &rdb){
    FieldMeterGraph::checkResources(rdb);

    // FIXME: document the color scheme
    setfieldcolor( 0, rdb.getResource( "cpuUserColor" ) );
    setfieldcolor( 1, rdb.getResource( "cpuNiceColor" ) );
    setfieldcolor( 2, rdb.getResource( "cpuWaitColor" ) );
    setfieldcolor( 3, rdb.getResource( "cpuSystemColor" ) );
    setfieldcolor( 4, rdb.getResource( "cpuFreeColor" ) );

    decayUsed(rdb.isResourceTrue("cpuUsedDecay"));
    priority_ = util::stoi (rdb.getResource( "cpuPriority" ));
    dodecay_ = rdb.isResourceTrue( "cpuDecay" );
    useGraph_ = rdb.isResourceTrue( "cpuGraph" );
    setUsedFormat (rdb.getResource("cpuUsedFormat"));
}


void CPUMeter::checkevent( void ){
    std::vector<uint64_t> ticks(getTicks());

    total_ = (float)(ticks[5] - _lastTicks[5]);
    fields_[0] = (float)(ticks[0] - _lastTicks[0]);
    fields_[1] = (float)(ticks[1] - _lastTicks[1]);
    fields_[2] = (float)(ticks[3] - _lastTicks[3]);
    fields_[3] = (float)(ticks[2] - _lastTicks[2]);
    fields_[4] = (float)(ticks[4] - _lastTicks[4]);
    setUsed(total_ - fields_[4], total_);
    _lastTicks = ticks;
    drawfields(parent_->g());
}


size_t CPUMeter::countCPUs(void){
    return 1;
}


std::string CPUMeter::cpuStr(size_t /* num */){
    return "cpu";
}


std::vector<uint64_t> CPUMeter::getTicks(void) const {
    // 0=usr, 1=nice, 2=sys, 3=kern, 4=idle, 5=total
    std::vector<uint64_t> rval(6, 0);
    std::vector<XOSVProc> procs(XOSVProc::ptable());

    for (size_t i = 0 ; i < procs.size() ; i++) {
        // FIXME: check about just using cycles
        uint64_t uticks = procs[i].cycles + procs[i].kipc_cycles
            + procs[i].kcall_cycles;
        rval[5] += uticks;

        if (procs[i].pid == IDLE)
            rval[4] += uticks;
        else if (procs[i].pid == KERNEL)
            rval[3] += uticks;
        else if (procs[i].ptype != TYPE_TASK) {
            if (procs[i].ptype == TYPE_SYSTEM)
                rval[2] += uticks;
            else {
                if (procs[i].nice >= 19)
                    rval[1] += uticks;
                else
                    rval[0] += uticks;
            }
        }
    }

    return rval;
}
