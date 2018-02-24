//
//  Copyright (c) 1994, 1995, 2006, 2015, 2016, 2017, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "intmeter.h"
#include "scache.h"
#include "strutil.h"

#include <fstream>
#include <limits>


static const char * const INTFILE = "/proc/interrupts";



IntMeter::IntMeter(size_t cpu)
    : ComIntMeter(makeTitle(cpu)), _cpu(cpu) {
    // if cpu == 0:  act as cummulative
    // if cpu >= 1:  meter for cpu - 1 (in /proc/interrupts)
}


std::string IntMeter::makeTitle(size_t cpu) const {
    // set the CPU in the title
    std::ostringstream os;
    os << "INT";
    if (cpu > 0) {
        os << "(";
        os << cpu - 1 << ")";
    }

    return os.str();
}


std::map<size_t, uint64_t> IntMeter::getStats(void) {

    static StatCache<std::vector<std::map<size_t, uint64_t> > > sc;
    if (!sc.valid())
        sc.set(readStats());

    return sc.get()[_cpu];
}


std::vector<std::map<size_t, uint64_t> > IntMeter::readStats(void) const {
    std::ifstream ifs(INTFILE);
    if (!ifs)
        logFatal << "could not open: " << INTFILE << std::endl;

    // First line is CPU0 CPU1 ... CPUN
    // Use it to size the return value.
    std::string ln;
    std::getline(ifs, ln);

    // size it one larger for the cummulative stats.
    std::vector<std::map<size_t, uint64_t> > rval(util::split(ln).size() + 1);

    // Each line is label: count0 count1 ... description.
    // Put the counts for labels that are numbers into the map for that cpu.
    while (!ifs.eof()) {
        size_t irq;
        ifs >> irq;
        if (ifs.fail())
            break; // the label was not a number.  No more numbers expected.

        std::string delim;
        ifs >> delim;
        if (delim != ":")
            logFatal << "failed to parse " << INTFILE << std::endl;

        // start at index 1 as 0 is for the sum of all cpus.
        for (size_t i = 1 ; i < rval.size() ; i++) {
            uint64_t count;
            ifs >> count;
            if (ifs.fail())
                logFatal << "failed to parse " << INTFILE << std::endl;
            rval[i][irq] = count;
            rval[0][irq] += count;
        }

        ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    return rval;
}
