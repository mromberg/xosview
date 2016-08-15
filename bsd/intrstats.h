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


class IntrStats {
public:
    IntrStats(void);

    // ----- old API -------
    size_t maxirq(void) const; // highest irq number.
    void stats(std::vector<uint64_t> &intrCount,
      std::vector<unsigned int> &intrNbrs) const;
    // ----- old API -------

    void scan(void); // find and cache "active" irqs.  somewhat expensive.

    std::map<size_t, uint64_t> counts(void) const;

    std::ostream &printOn(std::ostream &os) const;

private:
    std::map<size_t, size_t> _irqMap; // key=irq, val=index
};


inline std::ostream &operator<<(std::ostream &os, const IntrStats &is) {
    return is.printOn(os);
}


#endif
