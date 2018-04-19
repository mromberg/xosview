//
//  Copyright (c) 1994, 1995, 2006, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "timer.h"

#include <iostream>



Timer::Timer(bool start) {
    static_assert(tclock::is_steady,
      "Clock is not monotonically-increasing (steady).");

    if (start)
        Timer::start();
}


double Timer::report_usecs(void) const {
    return std::chrono::duration_cast<std::chrono::microseconds>(_stop
      - _start).count();
}


double Timer::report(void) const {
    return std::chrono::duration<double>(_stop - _start).count();
}


std::ostream &Timer::printOn(std::ostream &os) const {
    return os << "Timer : ["
              << "_startTime = " << _start.time_since_epoch().count() << ", "
              << "_stopTime = " << _stop.time_since_epoch().count() << ", "
              << "duration = " << report_usecs() << " usecs]";
}
