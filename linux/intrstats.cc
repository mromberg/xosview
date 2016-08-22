//
//  Copyright (c) 2016
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "intrstats.h"
#include "log.h"
#include "strutil.h"
#include "scache.h"

#include <fstream>


static const char * const INTFILE = "/proc/interrupts";


const std::vector<std::map<size_t, uint64_t> > &IntrStats::get(void) {

    static StatCache<std::vector<std::map<size_t, uint64_t> > > sc;

    if (!sc.valid())
        sc.set(read());

    return sc.get();
}


std::vector<std::map<size_t, uint64_t> > IntrStats::read(void) {

    std::ifstream ifs(INTFILE);
    if (!ifs)
        logFatal << "could not open: " << INTFILE << std::endl;

    // First line is CPU0 CPU1 ... CPUN
    // Use it to size the return value.
    std::string ln;
    std::getline(ifs, ln);

    std::vector<std::map<size_t, uint64_t> > rval(util::split(ln).size());

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

        uint64_t count;
        for (size_t i = 0 ; i < rval.size() ; i++) {
            ifs >> count;
            if (ifs.fail())
                logFatal << "failed to parse " << INTFILE << std::endl;
            rval[i][irq] = count;
        }


        ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    return rval;
}
