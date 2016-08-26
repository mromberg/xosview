//
//  Copyright (c) 1994, 1995, 2002, 2006, 2015, 2016
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

//  From the man page.  Listed here for reference.
//         user
//             (1) Time spent in user mode.
//         nice
//             (2) Time spent in user mode with low priority (nice).
//         system
//             (3) Time spent in system mode.
//         idle
//             (4) Time spent in the idle task.
//         iowait (since Linux 2.5.41)
//             (5) Time waiting for I/O to complete.
//         irq (since Linux 2.6.0-test4)
//             (6) Time servicing interrupts.
//         softirq (since Linux 2.6.0-test4)
//             (7) Time servicing softirqs.
//         steal (since Linux 2.6.11)
//             (8) Stolen time, which is the time spent in other
//                 operating systems when running in a virtualized environment
//         guest (since Linux 2.6.24)
//             (9) Time spent running a virtual CPU for guest operating
//                 systems under the control of the Linux kernel.
//         guest_nice (since Linux 2.6.33)
//             (10) Time spent running a niced guest (virtual CPU for guest
//                  operating systems under the control of the Linux kernel).

#include "cpumeter.h"
#include "scache.h"

#include <fstream>
#include <string>
#include <sstream>
#include <limits>


static const char * const STATFILENAME = "/proc/stat";



CPUMeter::CPUMeter(size_t cpu)
    : FieldMeterGraph( 10, util::toupper(CPUMeter::cpuStr(cpu)),
      "USR/NI/SYS/IO/I/SI/ST/GST/NGST/IDL"),
      _cpu(cpu), _oldStats(numfields(), 0) {

    getStats(_oldStats);
}


CPUMeter::~CPUMeter( void ){
}


void CPUMeter::checkResources(const ResDB &rdb){
    FieldMeterGraph::checkResources(rdb);

    setfieldcolor( 0, rdb.getColor( "cpuUserColor" ) );
    setfieldcolor( 1, rdb.getColor( "cpuNiceColor" ) );
    setfieldcolor( 2, rdb.getColor( "cpuSystemColor" ) );
    setfieldcolor( 3, rdb.getColor( "cpuWaitColor" ) );
    setfieldcolor( 4, rdb.getColor( "cpuInterruptColor" ) );
    setfieldcolor( 5, rdb.getColor( "cpuSoftIntColor" ) );
    setfieldcolor( 6, rdb.getColor( "cpuStolenColor" ) );
    setfieldcolor( 7, rdb.getColor( "cpuGuestColor" ) );
    setfieldcolor( 8, rdb.getColor( "cpuNiceGuestColor" ) );
    setfieldcolor( 9, rdb.getColor( "cpuFreeColor" ) );
}


void CPUMeter::checkevent( void ) {
    std::vector<uint64_t> cstats(numfields(), 0);
    getStats(cstats);

    total_ = 0;
    float used = 0;          // for setUsed
    unsigned int sindex = 0; // index into cstats
    float idle = 0;          // temp storage for idle field

    for (unsigned int i = 0 ; i < numfields() - 1; i++) {
        if (sindex == 3) { // idle field
            idle = cstats[sindex] - _oldStats[sindex]; // save for later
            total_ += idle;
            sindex++;  // offset
        }

        fields_[i] = cstats[sindex] - _oldStats[sindex];
        total_ += fields_[i];
        used += fields_[i];

        sindex++; // normal move ahead
    }
    fields_[numfields()-1] = idle;  // fill in the idle

    _oldStats = cstats;

    // Convert fields_ to percent
    if (total_ == 0.0)
        total_ = 1.0;
    used = used / total_;
    for (size_t i = 0 ; i < numfields() ; i++)
        fields_[i] = fields_[i] / total_;
    total_ = 1.0;

    setUsed(used, total_);
}


// Checks for the SMP kernel patch by forissier@isia.cma.fr.
// http://www-isia.cma.fr/~forissie/smp_kernel_patch/
// If it finds that this patch has been applied to the current kernel
// then returns the number of cpus that are on this machine.
size_t CPUMeter::countCPUs(void){
    static size_t cpuCount = 0;
    static bool first = true;

    if (first) {
        first = false;
        std::ifstream stats( STATFILENAME );

        if ( !stats )
            logFatal << "Can not open file : " << STATFILENAME << std::endl;

        std::string buf;
        while (getline(stats, buf))
            if ((buf.substr(0, 3) == "cpu") && buf[3] != ' ')
                cpuCount++;
    }

    return cpuCount;
}

std::string CPUMeter::cpuStr(size_t num){
    if (num == 0)  // The cumulative meter
        return "cpu";
    return std::string("cpu") + util::repr(num-1);
}


void CPUMeter::getStats(std::vector<uint64_t> &v) const {

    static StatCache<std::vector<std::vector<uint64_t> > > sc;

    if (!sc.valid())
        sc.set(readStats());

    v = sc.get()[_cpu];
}


std::vector<std::vector<uint64_t> > CPUMeter::readStats(void) const {
    // index0 = combined cpus, index1 = first cpu, ...
    std::vector<std::vector<uint64_t> > rval(countCPUs() + 1,
      std::vector<uint64_t>(numfields()));

    std::ifstream stats( STATFILENAME );
    if ( !stats )
        logFatal << "Can not open file : " << STATFILENAME << std::endl;

    while (!stats.eof()) {
        std::string idstr;
        stats >> idstr;
        if (stats.eof())
            break;
        if (stats.fail())
            logFatal << "failed to parse: " << STATFILENAME << std::endl;
        if (idstr.substr(0, 3) == "cpu") {
            size_t cpui = 0;
            if (idstr.size() > 3) {
                if (!util::fstr(idstr.substr(3), cpui))
                    logFatal << "failed to parse: " << STATFILENAME
                             << std::endl;
                cpui += 1;
            }
            size_t index = 0;
            while (index < numfields() && stats.peek() != '\n') {
                stats >> rval[cpui][index];
                if (stats.fail())
                    logFatal << "failed to parse: " << STATFILENAME
                             << std::endl;
                index++;
            }
            // skip any new additional stats on this line.
            stats.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        else
            break; // assumes cpu entries are at the top.
    }

    return rval;
}
