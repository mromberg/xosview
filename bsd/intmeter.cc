//
//  Copyright (c) 1994, 1995, 2015, 2016
//  by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  This file may be distributed under terms of the GPL
//

#include "intmeter.h"
#include "intrstats.h"



IntMeter::IntMeter(void)
    : BitMeter( "INTS", "IRQs" ), _irqcount(0) {

    _istats.scan();
    _irqcount = _istats.maxirq();
    _irqs.resize(_irqcount + 1, 0);
    _lastirqs.resize(_irqcount + 1, 0);
    _inbrs.resize(_irqcount + 1, false);

    updateirqcount(true);

    logDebug << "_istats: " << _istats << "\n"
             << "maxirq: " << _istats.maxirq() << std::endl;
}


IntMeter::~IntMeter( void ) {
}


void IntMeter::checkevent( void ) {
    getirqs();

    for (size_t i = 0 ; i <= _irqcount ; i++) {
        if (_inbrs[i]) {
            // new interrupt number
            if (_realintnum.find(i) == _realintnum.end()) {
                updateirqcount();
                return;
            }
            _bits[_realintnum[i]] = ((_irqs[i] - _lastirqs[i]) != 0);
            _lastirqs[i] = _irqs[i];
        }
    }
    for (size_t i = 0 ; i < _irqcount + 1 ; i++) {
        _inbrs[i] = false;
        _irqs[i] = 0;
    }
}


void IntMeter::checkResources(const ResDB &rdb) {
    BitMeter::checkResources(rdb);
    _dbits.color(true, rdb.getColor("intOnColor"));
    _dbits.color(false, rdb.getColor("intOffColor"));
}


void IntMeter::getirqs( void ) {

    const size_t intVectorLen = _istats.maxirq() + 1;
    _irqs.resize(intVectorLen);
    _inbrs.resize(intVectorLen, false);

    const std::map<size_t, uint64_t> &cmap = _istats.counts();

    for (size_t i = 0 ; i < _irqs.size() ; i++) {
        std::map<size_t, uint64_t>::const_iterator it = cmap.find(i);
        if (it != cmap.end()) {
            _irqs[i] = it->second;
            _inbrs[i] = true;
        }
        else {
            _irqs[i] = 0;
            _inbrs[i] = false;
        }
    }
}


void IntMeter::updateirqcount( bool init ) {
    int count = 16;

    if (init) {
        getirqs();
        for (int i = 0; i < 16; i++)
            _realintnum[i] = i;
    }
    for (size_t i = 16; i <= _irqcount; i++) {
        if (_inbrs[i]) {
            _realintnum[i] = count++;
            _inbrs[i] = false;
        }
    }
    setNumBits(count);

    // Build the legend.
    std::ostringstream os;
    os << "0";
    if (_realintnum.upper_bound(15) == _realintnum.end()) // only 16 ints
        os << "-15";
    else {
        int prev = 15, prev2 = 14;
        for (std::map<int,int>::const_iterator it = _realintnum.upper_bound(15),
                 end = _realintnum.end(); it != end; ++it) {
            if ( &*it == &*_realintnum.rbegin() ) { // last element
                if (it->first == prev + 1)
                    os << "-" ;
                else {
                    if (prev == prev2 + 1)
                        os << "-" << prev;
                    os << "," ;
                }
                os << it->first;
            }
            else {
                if (it->first != prev + 1) {
                    if (prev == prev2 + 1)
                        os << "-" << prev;
                    os << "," << it->first ;
                }
            }
            prev2 = prev;
            prev = it->first;
        }
        os << std::ends;
    }
    legend(os.str().c_str());
}
