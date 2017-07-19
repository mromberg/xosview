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

//--------------------------------------------------------------
// The minix kernel does not keep track of the cycles used on
// a per cpu basis.  The stat is a total per process.  So,
// xosview (and top) have to keep a process table around in
// memory and then do a bunch of math to figure out how many
// cycles the cpu has used.
//
// It might be a fun project to find where the per process
// counts are being incremented in the kernel and add one line
// of code to increment a counter for the cpu.  Then xosview
// and top would not have to do this.
//--------------------------------------------------------------


CPUMeter::CPUMeter(unsigned int cpu)
    : FieldMeterGraph( 7, util::toupper(CPUMeter::cpuStr(cpu)),
      "USR/NI/KERN/KIPC/KCALL/SYS/IDL"), _cpu(cpu) {

    // Suppress the warning about _cpu being unused.  In the future
    // minix may support smp and we will use it for real.
    (void)_cpu;

    getTicks(); // initialize _ptable
}


CPUMeter::~CPUMeter( void ){
}


void CPUMeter::checkResources(const ResDB &rdb){
    FieldMeterGraph::checkResources(rdb);

    setfieldcolor( 0, rdb.getColor( "cpuUserColor" ) );
    setfieldcolor( 1, rdb.getColor( "cpuNiceColor" ) );
    setfieldcolor( 2, rdb.getColor( "cpuSystemColor" ) );
    setfieldcolor( 3, rdb.getColor( "cpuWaitColor" ) );
    setfieldcolor( 4, rdb.getColor( "cpuSoftIntColor" ) );
    setfieldcolor( 5, rdb.getColor( "cpuInterruptColor" ) );
    setfieldcolor( 6, rdb.getColor( "cpuFreeColor" ) );
}


void CPUMeter::checkevent( void ){

    std::vector<uint64_t> ticks(getTicks());

    //logDebug << ticks << std::endl;

    float ticktotal = (float)ticks[7];

    for (size_t i = 0 ; i + 1 < ticks.size() ; i++)
        fields_[i] = (float)ticks[i] / ticktotal;

    total_ = 1.0;
    setUsed(total_ - fields_[6], total_);
}


size_t CPUMeter::countCPUs(void){
    return 1;
}


std::string CPUMeter::cpuStr(size_t /* num */){
    return "cpu";
}


std::vector<uint64_t> CPUMeter::getTicks(void) {

    // 0=usr, 1=nice, 2=sys, 3=kern, 4=kipc, 5=kcall, 6=idle, 7=total
    std::vector<uint64_t> rval(8, 0);
    std::vector<XOSVProc> procs(XOSVProc::ptable());
    std::map<pid_t, XOSVProc> nptable;

    for (size_t i = 0 ; i < procs.size() ; i++) {

        // get current total ticks, subtract last count
        // and save new count.  Build new ptable (in case a process
        // goes away).
        pid_t pid = procs[i].pid;
        uint64_t uticks = procs[i].execycles;
        uint64_t kiticks = procs[i].kerncycles;
        uint64_t kcticks = procs[i].kcallcycles;
        const XOSVProc &last = _ptable[pid];
        nptable[pid] = procs[i];
        uticks -= last.execycles;
        kiticks -= last.kerncycles;
        kcticks -= last.kcallcycles;

        rval[4] += kiticks;
        rval[5] += kcticks;
        rval[7] += uticks + kiticks + kcticks;

        // put the execycles into a bin based on ptype.
        if (procs[i].ptype == TYPE_TASK) {
            if (procs[i].pid == IDLE)
                rval[6] += uticks; // idle
            else if (procs[i].pid == KERNEL)
                rval[3] += uticks; // kernel
        }
        else {
            if (procs[i].ptype == TYPE_SYSTEM)
                rval[2] += uticks; // sys
            else {
                if (procs[i].nice >= 19)
                    rval[1] += uticks; // nice
                else
                    rval[0] += uticks; // user
            }
        }
    }

    _ptable = nptable;

    return rval;
}
