//
//  Copyright (c) 2016
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef intrstats_h
#define intrstats_h


#include <map>
#include <vector>
#include <iostream>



class IntrStats {
public:
    IntrStats(void);

    void scan(void); // find and cache "active" irqs.  somewhat expensive.
    size_t maxirq(void) const; // highest irq number.
    const std::map<size_t, uint64_t> &counts(void) const;

    std::ostream &printOn(std::ostream &os) const;

private:
    std::map<size_t, size_t> _irqMap; // key=irq, val=index

    std::map<size_t, uint64_t> readCounts(void) const;

    template <class X>
    std::map<size_t, uint64_t> getCounts(const std::vector<X> &v) const;
};


inline std::ostream &operator<<(std::ostream &os, const IntrStats &is) {
    return is.printOn(os);
}


template <class X>
std::map<size_t, uint64_t> IntrStats::getCounts(const std::vector<X> &v) const {
    std::map<size_t, uint64_t> rval;

    std::map<size_t, size_t>::const_iterator it;
    for (it = _irqMap.begin() ; it != _irqMap.end() ; ++it)
        rval[it->first] = v[it->second];

    return rval;
}


#endif
