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
#include "scache.h"

#if defined(XOSVIEW_NETBSD)
#include <sys/evcnt.h>
#endif


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

    const std::map<size_t, uint64_t> &cmap = counts();

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


const std::map<size_t, uint64_t> &IntrStats::counts(void) const {
    static StatCache<std::map<size_t, uint64_t> > sc;

    if (!sc.valid())
        sc.set(readCounts());

    return sc.get();
}


std::ostream &IntrStats::printOn(std::ostream &os) const {
    return os << "irqMap=" << _irqMap;
}


#if defined(XOSVIEW_OPENBSD)
void IntrStats::scan(void) {
    const int mib_nint[] = { CTL_KERN, KERN_INTRCNT, KERN_INTRCNT_NUM };
    static SysCtl nintr_sc(mib_nint, 3);
    const int mib_intc[] = { CTL_KERN, KERN_INTRCNT, KERN_INTRCNT_CNT, 0 };
    static SysCtl intrcnt_sc(mib_intc, 4);
    //const int mib_intn[] = { CTL_KERN, KERN_INTRCNT, KERN_INTRCNT_NAME, 0 };
    //static SysCtl intrnam_sc(mib_intn, 4);

    int nintr = 0;
    if (!nintr_sc.get(nintr))
        logFatal << "sysctl(" << nintr_sc.id() << ") failed." << std::endl;

    for (int i = 0 ; i < nintr ; i++) {
        intrcnt_sc.mib()[3] = i;

        uint64_t count = 0;
        if (!intrcnt_sc.get(count))
            continue;  // not active.

        _irqMap[i] = i;
    }
}
#endif


#if defined(XOSVIEW_OPENBSD)
std::map<size_t, uint64_t> IntrStats::readCounts(void) const {
    const int mib_intc[] = { CTL_KERN, KERN_INTRCNT, KERN_INTRCNT_CNT, 0 };
    static SysCtl intrcnt_sc(mib_intc, 4);

    std::map<size_t, uint64_t> rval;

    std::map<size_t, size_t>::const_iterator it;
    for (it = _irqMap.begin() ; it != _irqMap.end() ; ++it) {
        intrcnt_sc.mib()[3] = it->second;
        uint64_t count = 0;
        if (!intrcnt_sc.get(count))
            logFatal << "sysctl(" << intrcnt_sc.id() << ") failed."
                     << std::endl;
        rval[it->first] = count;
    }

    return rval;
}
#endif


// Note: combine scan() and counts() using lambdas when C++11 support
//       can be assumed to be widely supported.
#if defined(XOSVIEW_NETBSD)
void IntrStats::scan(void) {
    const int Mib[] = { CTL_KERN, KERN_EVCNT, EVCNT_TYPE_INTR,
                        KERN_EVCNT_COUNT_ANY };
    static SysCtl evcnt_sc(Mib, sizeof(Mib) / sizeof(int));

    size_t evsize = 0;
    if (!evcnt_sc.getsize(evsize))
        logFatal << "sysctl(" << evcnt_sc.id() << ") failed." << std::endl;

    std::vector<char> buf(evsize, 0);
    if (!evcnt_sc.get(buf))
        logFatal << "sysctl(" << evcnt_sc.id() << ") failed." << std::endl;

    const struct evcnt_sysctl *evs =
        reinterpret_cast<const struct evcnt_sysctl *>(buf.data());
    const struct evcnt_sysctl *evsend =
        reinterpret_cast<const struct evcnt_sysctl *>(buf.data() + buf.size());

    size_t i = 0;
    while (evs->ev_len && evs < evsend &&
      reinterpret_cast<const struct evcnt_sysctl *>(
          (char *)evs + evs->ev_len * 8) < evsend ) {

        // ------- lambda -----
        // extract the "pin" number from the name.
        std::string name(evs->ev_strings + evs->ev_grouplen + 1);
        size_t nbr = 0;
        std::string dummy;
        std::istringstream is(name);
        is >> dummy >> nbr;

        if (is)
            _irqMap[nbr] = i;
        // ------- lambda -----

        evs = reinterpret_cast<const struct evcnt_sysctl *>((const char *)evs
          + 8 * evs->ev_len);
        i++;
    }
}
#endif


#if defined(XOSVIEW_NETBSD)
std::map<size_t, uint64_t> IntrStats::readCounts(void) const {
    const int Mib[] = { CTL_KERN, KERN_EVCNT, EVCNT_TYPE_INTR,
                        KERN_EVCNT_COUNT_ANY };
    static SysCtl evcnt_sc(Mib, sizeof(Mib) / sizeof(int));

    size_t evsize = 0;
    if (!evcnt_sc.getsize(evsize))
        logFatal << "sysctl(" << evcnt_sc.id() << ") failed." << std::endl;

    std::vector<char> buf(evsize, 0);
    if (!evcnt_sc.get(buf))
        logFatal << "sysctl(" << evcnt_sc.id() << ") failed." << std::endl;

    const struct evcnt_sysctl *evs =
        reinterpret_cast<const struct evcnt_sysctl *>(buf.data());
    const struct evcnt_sysctl *evsend =
        reinterpret_cast<const struct evcnt_sysctl *>(buf.data() + buf.size());

    std::map<size_t, uint64_t> rval;
    size_t i = 0;
    while (evs->ev_len && evs < evsend &&
      reinterpret_cast<const struct evcnt_sysctl *>(
          (char *)evs + evs->ev_len * 8) < evsend ) {

        // ------- lambda -----
        std::map<size_t, size_t>::const_iterator it = _irqMap.find(i);
        if (it != _irqMap.end())
            rval[i] = evs->ev_count;
        // ------- lambda -----

        evs = reinterpret_cast<const struct evcnt_sysctl *>((const char *)evs
          + 8 * evs->ev_len);
        i++;
    }

    return rval;
}
#endif


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
std::map<size_t, uint64_t> IntrStats::readCounts(void) const {

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
