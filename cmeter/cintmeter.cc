//
//  Copyright (c) 2017, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "cintmeter.h"
#include "strutil.h"


static const size_t ResIRQ = 15; // reserved irqs (always show 0 - ResIRQ bits)



ComIntMeter::ComIntMeter(const std::string &title)
    : BitMeter(title, "") {
}

void ComIntMeter::checkevent(void) {
    const std::map<size_t, uint64_t> newc = getStats();
    auto itn = newc.cbegin();
    auto itl = _last.begin();

    while (itn != newc.cend() && itl != _last.end()) {
        if (itn->first != itl->first) {
            _last = newc;
            break;
        }
        auto itim = _imap.find(itn->first);
        if (itim == _imap.end())
            break;  // maybe also initUI()?
        _bits[itim->second] = itn->second != itl->second;
        itl->second = itn->second;
        ++itn;
        ++itl;
    }
}


void ComIntMeter::checkResources(const ResDB &rdb) {
    BitMeter::checkResources(rdb);
    _dbits.color(0, rdb.getColor("intOffColor"));
    _dbits.color(1, rdb.getColor("intOnColor"));

    _last = getStats();
    logDebug << "_last: " << _last << "\n"
             << "irqCount: " << _last.size() << std::endl;

    initUI();
}


void ComIntMeter::initIMap(void) {
    _imap.clear();

    for (size_t i = 0 ; i <= ResIRQ ; i++)
        _imap[i] = i;

    size_t next = ResIRQ + 1;
    for (auto it = _last.cbegin() ; it != _last.cend() ; ++it) {
        if (it->first > ResIRQ)
            _imap[it->first] = next++;
    }
}


void ComIntMeter::initUI(void) {
    // recalc the irq -> bit index map.
    initIMap();

    // parent handles the bit display
    const size_t nbits = _imap.empty() ? 1 : _imap.size();
    setNumBits(nbits);

    // set the legend
    legend(makeLegend());
}


std::string ComIntMeter::makeLegend(void) const {
    std::ostringstream ostr;

    ostr << "IRQS: (";
    size_t last = -1;
    bool inrange = false;
    for (auto it = _imap.cbegin() ; it != _imap.cend() ; ++it) {
        if (last == static_cast<size_t>(-1))
            ostr << it->first;
        else {
            if (it->first == last + 1) {
                if (!inrange) {
                    inrange = true;
                    ostr << "-";
                }
            }
            else {
                if (inrange) {
                    inrange = false;
                    ostr << last;
                }
                ostr << "," << it->first;
            }
        }
        last = it->first;
    }
    if (inrange)
        ostr << last;
    ostr << ")";

    return ostr.str();
}
