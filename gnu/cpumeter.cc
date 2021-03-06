//
//  Copyright (c) 1994, 1995, 2002, 2006, 2015, 2018
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

#include <fstream>


static const char * const STATFILENAME = "/proc/stat";
static const char * const LEGEND = "USR/NI/SYS/IO/I/SI/ST/GST/NGST/IDL";
static const size_t MAX_PROCSTAT_LENGTH = 4096;


CPUMeter::CPUMeter(size_t cpu)
    : FieldMeterGraph(10, util::toupper(CPUMeter::cpuStr(cpu)), LEGEND),
      _cpu(cpu), _lineNum(findLine()), _oldStats(numfields()) {
    logDebug << "CPUMeter::CPUMeter(" << cpu << ")" << std::endl;
    getStats(_oldStats);
}


void CPUMeter::checkResources(const ResDB &rdb){
    FieldMeterGraph::checkResources(rdb);

    setfieldcolor(0, rdb.getColor( "cpuUserColor"));
    setfieldcolor(1, rdb.getColor( "cpuNiceColor"));
    setfieldcolor(2, rdb.getColor( "cpuSystemColor"));
    setfieldcolor(3, rdb.getColor( "cpuWaitColor"));
    setfieldcolor(4, rdb.getColor( "cpuInterruptColor"));
    setfieldcolor(5, rdb.getColor( "cpuSoftIntColor"));
    setfieldcolor(6, rdb.getColor( "cpuStolenColor"));
    setfieldcolor(7, rdb.getColor( "cpuGuestColor"));
    setfieldcolor(8, rdb.getColor( "cpuNiceGuestColor"));
    setfieldcolor(9, rdb.getColor( "cpuFreeColor"));
}


void CPUMeter::checkevent(void) {
    getcputime();
}


void CPUMeter::getcputime(void) {
    std::vector<unsigned long long> cstats(numfields(), 0);
    getStats(cstats);

    _total = 0;
    size_t sindex = 0; // index into cstats
    float used = 0;    // for setUsed
    float idle = 0;    // temp storage for idle field

    for (size_t i = 0 ; i + 1 < numfields() ; i++) {
        if (sindex == 3) { // idle field
            idle = cstats[sindex] - _oldStats[sindex]; // save for later
            _total += idle;
            sindex++;  // offset
        }

        _fields[i] = cstats[sindex] - _oldStats[sindex];
        _total += _fields[i];
        used += _fields[i];

        sindex++; // normal move ahead
    }
    _fields[numfields()-1] = idle;  // fill in the idle

    _oldStats = cstats;

    // Convert _fields to percent
    if (_total == 0.0)
        _total = 1.0;
    used = used / _total;
    for (auto &field : _fields)
        field /= _total;
    _total = 1.0;

    setUsed(used, _total);
}


size_t CPUMeter::findLine(void) const {
    std::ifstream stats(STATFILENAME);
    if (!stats)
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
size_t CPUMeter::countCPUs(void) {
    static size_t cpuCount = 0;
    static bool first = true;

    if (first) {
        first = false;
        std::ifstream stats(STATFILENAME);

        if (!stats)
            logFatal << "Can not open file : " << STATFILENAME << std::endl;

        std::string buf;
        while (getline(stats, buf))
            if ((buf.substr(0, 3) == "cpu") && buf[3] != ' ')
                cpuCount++;
    }

    return cpuCount;
}


std::string CPUMeter::cpuStr(size_t num) {
    if (num == 0)  // The cumulative meter
        return "cpu";
    return std::string("cpu") + util::repr(num - 1);
}


void CPUMeter::getStats(std::vector<unsigned long long> &statv) const {
    std::ifstream stats(STATFILENAME);
    if (!stats)
        logFatal << "Can not open file : " << STATFILENAME << std::endl;

    // read until we are at the right line.
    for (size_t i = 0 ; i < _lineNum ; i++) {
        if (stats.eof())
            break;
        stats.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    // Parse the line for this cpu
    std::string tmp; // The cpuID
    stats >> tmp;

    for (size_t i = 0 ; i < statv.size() ; i++) {
        stats >> statv[i];
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
}
