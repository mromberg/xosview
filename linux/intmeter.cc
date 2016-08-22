//
//  Copyright (c) 1994, 1995, 2006, 2015, 2016
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "intmeter.h"



IntMeter::IntMeter(size_t cpu)
    : BitMeter( "INT", "" ), _cpu(cpu) {
    // if cpu == 0:  act as cummulative
    // if cpu >= 1:  meter for cpu - 1 (in /proc/interrupts)

    _last = _istats.readCounts(_cpu);

    logDebug << "_last: " << _last << std::endl;
    logDebug << "IntMeter::IntMeter() cpu: " << _cpu << "\n";
    logDebug << "irqCount: " << _last.size() << std::endl;

    initUI();
}


IntMeter::~IntMeter( void ){
}


void IntMeter::checkevent( void ){
    std::vector<uint64_t> newc = _istats.readCounts(_cpu);
    for (size_t i = 0 ; i < numBits() ; i++) {
        _bits[i] = newc[i] - _last[i];
    }
    _last = newc;
}


void IntMeter::checkResources(const ResDB &rdb) {
    BitMeter::checkResources(rdb);
    _dbits.color(0, rdb.getColor("intOffColor"));
    _dbits.color(1, rdb.getColor("intOnColor"));
}


void IntMeter::initUI(void) {
    // parent handles the bit display
    setNumBits(_last.size());

    // set the legend
    std::string lgnd("IRQS: (0-");
    lgnd += util::repr(_last.size() - 1) + ")";
    legend(lgnd);

    // set the CPU in the title
    std::ostringstream os;
    os << "INT";
    if (_cpu != 0) {
        os << "(";
        os << _cpu - 1 << ")";
    }
    title(os.str());
}
