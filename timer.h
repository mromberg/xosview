//
//  Copyright (c) 1994, 1995, 2006, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef timer_h
#define timer_h

//
//                 General purpose interval timer class
//
//   Author : Mike Romberg

#include <chrono>
#include <iosfwd>



class Timer {
public:
    Timer(bool start=false);

    void start(void) { _start = tclock::now(); }
    void stop(void) { _stop = tclock::now(); }

    //  reports time intervall between calls to start and stop in usec
    //  This one uses doubles as the return value, to avoid
    //  overflow/sign problems.
    double report_usecs(void) const;

    // in seconds.
    double report(void) const;

    std::ostream &printOn(std::ostream &os) const;

private:
    // Try and find the most suitable clock.
    using tclock = std::conditional<
        std::chrono::high_resolution_clock::is_steady,
        std::chrono::high_resolution_clock,
        std::chrono::steady_clock>::type;

    std::chrono::time_point<tclock> _start, _stop;
};


inline std::ostream &operator<<(std::ostream &os, const Timer &t) {
    return t.printOn(os);
}

#endif
