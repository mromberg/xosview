//
//  Copyright (c) 1994, 1995, 2015, 2016
//  by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  This file may be distributed under terms of the GPL
//

#include "intmeter.h"
#include "intrstats.h"



IntMeter::IntMeter(void) : BitMeter( "INTS", "IRQs" ) {

    updateirqcount(true);
    _lastirqs = _istats.counts();

    logDebug << "_istats: " << _istats << "\n"
             << "maxirq: " << _istats.maxirq() << std::endl;
}


IntMeter::~IntMeter( void ) {
}


void IntMeter::checkevent( void ) {

    const std::map<size_t, uint64_t> &cmap = _istats.counts();

    std::map<size_t, uint64_t>::const_iterator it;
    for (it = cmap.begin() ; it != cmap.end() ; ++it) {
        // is the bit on or off?
        bool on = it->second - util::get(_lastirqs, it->first);

        // Find the index into _bits.  If not found update and bailout.
        std::map<size_t, size_t>::const_iterator bit = _irq2bit.find(it->first);

        if (bit != _irq2bit.end())
            _bits[bit->second] = on;
        else {
            updateirqcount();
            return;
        }
    }

    _lastirqs = cmap;
}


void IntMeter::checkResources(const ResDB &rdb) {
    BitMeter::checkResources(rdb);
    _dbits.color(true, rdb.getColor("intOnColor"));
    _dbits.color(false, rdb.getColor("intOffColor"));
}


void IntMeter::updateirqcount( bool init ) {
    _istats.scan();
    const std::map<size_t, uint64_t> &cmap = _istats.counts();

    size_t count = 16;

    if (init) {
        for (size_t i = 0; i < 16; i++)
            _irq2bit[i] = i;
    }
    std::map<size_t, uint64_t>::const_iterator it;
    for (it = cmap.begin() ; it != cmap.end() ; ++it)
        if (it->first > 15)
            _irq2bit[it->first] = count++;

    setNumBits(count);

    // Build the legend.
    std::ostringstream os;
    os << "0";
    if (_irq2bit.upper_bound(15) == _irq2bit.end()) // only 16 ints
        os << "-15";
    else {
        size_t prev = 15, prev2 = 14;
        std::map<size_t, size_t>::const_iterator it, end;
        for (it = _irq2bit.upper_bound(15), end = _irq2bit.end() ;
             it != end ; ++it) {

            if ( &*it == &*_irq2bit.rbegin() ) { // last element
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
    }
    legend(os.str());
}
