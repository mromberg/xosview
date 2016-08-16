//
//  Copyright (c) 2016
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "intrstats.h"
#include "log.h"
#include "sctl.h"
#include "strutil.h"


IntrStats::IntrStats(void) {
}


size_t IntrStats::maxirq(void) const {
    size_t rval = 0;

    if (!_irqMap.empty())
        rval = _irqMap.rbegin()->first;

    return rval;
}


void IntrStats::stats(std::vector<uint64_t> &intrCount,
  std::vector<unsigned int> &intrNbrs) const {

    size_t intVectorLen = maxirq() + 1;
    intrCount.resize(intVectorLen);
    intrNbrs.resize(intVectorLen);

    std::map<size_t, uint64_t> cmap = counts();

    for (size_t i = 0 ; i < intrCount.size() ; i++) {
        std::map<size_t, uint64_t>::const_iterator it = cmap.find(i);
        if (it != cmap.end()) {
            intrCount[i] = it->second;
            intrNbrs[i] = 1;
        }
        else {
            intrCount[i] = 0;
            intrNbrs[i] = 0;
        }
    }
}


std::ostream &IntrStats::printOn(std::ostream &os) const {
    return os << "irqMap=" << _irqMap;
}


#if defined(XOSVIEW_DFBSD)
void IntrStats::scan(void) {

    static SysCtl intrnames_sc("hw.intrnames");

    size_t bytes = 0;
    if (!intrnames_sc.getsize(bytes))
        logFatal << "sysctl(" << intrnames_sc.id() << ") failed." << std::endl;

    std::vector<char> nbuf(bytes, 0);
    if (!intrnames_sc.get(nbuf))
        logFatal << "sysctl(" << intrnames_sc.id() << ") failed." << std::endl;

    size_t i = 0, offset = 0;
    while (offset < nbuf.size()) {
        // unused ints are named irqn where
        // 0<=n<=255, used ones have device name
        std::string tstr(nbuf.data() + offset);
        if (tstr.substr(0, 3) != "irq")
            _irqMap[i] = i;

        offset += tstr.size() + 1;
        i++;
    }
}
#endif


#if defined(XOSVIEW_DFBSD) || defined(XOSVIEW_FREEBSD)
std::map<size_t, uint64_t> IntrStats::counts(void) const {

    static SysCtl intrcnt_sc("hw.intrcnt");

    size_t csize = 0;
    if (!intrcnt_sc.getsize(csize))
        logFatal << "sysctl(" << intrcnt_sc.id() << ") failed." << std::endl;

    std::vector<unsigned long> intrcnt(csize / sizeof(unsigned long));
    if (!intrcnt_sc.get(intrcnt))
        logFatal << "sysctl(" << intrcnt_sc.id() << ") failed." << std::endl;

    return getCounts(intrcnt);
}
#endif


#if defined(XOSVIEW_FREEBSD)
void IntrStats::scan(void) {
    static SysCtl intrnames_sc("hw.intrnames");

    size_t nsize = 0;
    if (!intrnames_sc.getsize(nsize))
        logFatal << "sysctl(" << intrnames_sc.id() << ") failed." << std::endl;

    // this thing will be 200kb or more!
    // So, make it static and grow it if needed.
    static std::vector<char> nbuf(nsize, 0);
    if (nbuf.size() < nsize)
        nbuf.resize(nsize);

    if (!intrnames_sc.get(nbuf))
        logFatal << "sysctl(" << intrnames_sc.id() << ") failed." << std::endl;

    size_t index = 0;
    const char *p = nbuf.data();
    // executive decision.  Stop at first null string
    // so we don't check 200k empty ones.
    while (*p && p < nbuf.data() + nbuf.size()) {
        std::string iname(p);
        size_t nbr = 0;
        if (iname.substr(0, 3) == "irq" && util::fstr(iname.substr(3), nbr))
            _irqMap[nbr] = index;

        p += iname.size() + 1;
        index++;
    }
}
#endif
