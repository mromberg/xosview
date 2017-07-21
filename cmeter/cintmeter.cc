#include "cintmeter.h"


static const size_t RESIRQ = 16; // reserved irqs (always show 0 - RESIRQ bits)



ComIntMeter::ComIntMeter(const std::string &title)
    : BitMeter(title, "") {
}

void ComIntMeter::checkevent(void) {
    const std::map<size_t, uint64_t> newc = getStats();
    std::map<size_t, uint64_t>::const_iterator itn = newc.begin();
    std::map<size_t, uint64_t>::iterator itl = _last.begin();

    while (itn != newc.end() && itl != _last.end()) {
        if (itn->first != itl->first) {
            _last = newc;
            break;
        }
        std::map<size_t, size_t>::const_iterator itim = _imap.find(itn->first);
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

    for (size_t i = 0 ; i <= RESIRQ ; i++)
        _imap[i] = i;

    size_t next = RESIRQ + 1;
    std::map<size_t, uint64_t>::const_iterator it;
    for (it = _last.begin() ; it != _last.end() ; ++it) {
        if (it->first > RESIRQ)
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
    std::map<size_t, size_t>::const_iterator it;
    for (it = _imap.begin() ; it != _imap.end() ; ++it) {
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
