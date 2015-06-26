//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef TIMEVAL_H
#define TIMEVAL_H

#include <iostream>

#include <sys/time.h>


class TimeVal {
public:
    TimeVal(unsigned long sec = 0, unsigned long usec = 0) {
        _val.tv_sec = (int)sec;
        _val.tv_usec = usec;
    }
    TimeVal(const struct timeval &val) { _val = val; }

    unsigned long sec(void) const { return _val.tv_sec; }
    unsigned long usec(void) const { return _val.tv_usec; }
    void sec(unsigned long s) { _val.tv_sec = (int)s; }
    void usec(unsigned long us) { _val.tv_usec = us; }

    operator struct timeval(void) const { return _val; }

    std::ostream &printOn(std::ostream &os) const {
        return os <<"(" <<sec() <<" sec, " <<usec() <<" usec)";
    }

private:
    struct timeval _val;
};

inline std::ostream &operator<<(std::ostream &os, const TimeVal &tv){
    return tv.printOn(os);
}

#endif
