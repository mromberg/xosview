//
//  Copyright (c) 1994, 1995, 2002, 2006, 2015
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
#include "xosview.h"
#include <fstream>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <limits>

static const char STATFILENAME[] = "/proc/stat";
static const size_t MAX_PROCSTAT_LENGTH = 4096;


CPUMeter::CPUMeter(XOSView *parent, unsigned int cpu)
    : FieldMeterGraph( parent, 10, util::toupper(CPUMeter::cpuStr(cpu)),
      "USR/NI/SYS/IO/I/SI/ST/GST/NGST/IDL"), _cpu(cpu) {
    _oldStats.resize(numfields());
    _lineNum = findLine();
    getStats(_oldStats);
}

CPUMeter::~CPUMeter( void ){
}

void CPUMeter::checkResources( void ){
    FieldMeterGraph::checkResources();

    setfieldcolor( 0, parent_->getResource( "cpuUserColor" ) );
    setfieldcolor( 1, parent_->getResource( "cpuNiceColor" ) );
    setfieldcolor( 2, parent_->getResource( "cpuSystemColor" ) );
    setfieldcolor( 3, parent_->getResource( "cpuWaitColor" ) );
    setfieldcolor( 4, parent_->getResource( "cpuInterruptColor" ) );
    setfieldcolor( 5, parent_->getResource( "cpuSoftIntColor" ) );
    setfieldcolor( 6, parent_->getResource( "cpuStolenColor" ) );
    setfieldcolor( 7, parent_->getResource( "cpuGuestColor" ) );
    setfieldcolor( 8, parent_->getResource( "cpuNiceGuestColor" ) );
    setfieldcolor( 9, parent_->getResource( "cpuFreeColor" ) );

    decayUsed(parent_->isResourceTrue("cpuUsedDecay"));
    priority_ = util::stoi (parent_->getResource( "cpuPriority" ));
    dodecay_ = parent_->isResourceTrue( "cpuDecay" );
    useGraph_ = parent_->isResourceTrue( "cpuGraph" );
    setUsedFormat (parent_->getResource("cpuUsedFormat"));
}

void CPUMeter::checkevent( void ){
    getcputime();
    drawfields(parent_->g());
}

void CPUMeter::getcputime( void ){
    std::vector<unsigned long long> cstats(numfields(), 0);
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

size_t CPUMeter::findLine(void) {
    std::ifstream stats(STATFILENAME);

    if ( !stats )
        logFatal << "Can not open file : " << STATFILENAME << std::endl;

    std::string cpuID(CPUMeter::cpuStr(_cpu));
    // make the ws part of the id to tell cpu1 from cpu11
    cpuID += " ";
    size_t line = 0;
    std::string buf;
    while (!stats.eof()){
        getline(stats, buf);
        if (!stats.eof()){
            if ((cpuID == buf.substr(0, cpuID.size()))) {
                return line;
            }
        }
        line++;
    }
    logFatal << "Failed to find " << cpuID
             << " in " << STATFILENAME << std::endl;
    return 0;
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

void CPUMeter::getStats(std::vector<unsigned long long> &v) const {
    std::ifstream stats( STATFILENAME );
    if ( !stats )
        logFatal << "Can not open file : " << STATFILENAME << std::endl;

    // read until we are at the right line.
    for (unsigned int i = 0 ; i < _lineNum ; i++) {
        if (stats.eof())
            break;
        stats.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    // Parse the line for this cpu
    std::string tmp; // The cpuID
    stats >> tmp;
    for (size_t i = 0 ; i < v.size() ; i++) {
        stats >> v[i];
        if (stats.fail()) {
            // kernels up to 2.5.41 (not including) have usr/ni/sys/idle
            // Die if we don't have all of these
            if (i < 4)
                logFatal << "error reading cpu stats from: " << STATFILENAME
                         << std::endl;
            // Stats added in 2.5.41 (and later).  Just use zero fields
            break;
        }
    }

    if (!stats)
        logBug << "error parsing: " << STATFILENAME << std::endl;
}
