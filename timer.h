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
//  Implemented using BSD derived function gettimeofday for greater resolution
//
//   Author : Mike Romberg

#include <iostream>

#include <sys/time.h>


inline std::ostream &operator<<(std::ostream &os, const struct timeval &tv) {
    return os << "(" << tv.tv_sec << ", " << tv.tv_usec << ")";
}


class Timer {
public:
    Timer(bool start=false) {
        // in C++11 you can do _startTime({0, 0}) in the initializer list
        _startTime.tv_sec = _startTime.tv_usec = 0;
        _stopTime.tv_sec = _stopTime.tv_usec = 0;
        if (start)
            Timer::start();
    }

    void start(void) { gettimeofday(&_startTime, nullptr); }
    void stop(void) { gettimeofday(&_stopTime, nullptr);  }
    //  reports time intervall between calls to start and stop in usec
    //  This one uses doubles as the return value, to avoid
    //  overflow/sign problems.
    double report_usecs(void) const {
        return (_stopTime.tv_sec - _startTime.tv_sec) * 1000000.0
            + _stopTime.tv_usec - _startTime.tv_usec;
    }

    double report(void) const { return report_usecs() / 1000000.0; } // sec

    std::ostream &printOn(std::ostream &os) const {
        return os << "Timer : ["
                  << "_startTime = " << _startTime << ", "
                  << "_stopTime = " << _stopTime << ", "
                  << "duration = " << report_usecs() <<" usecs]";
    }

private:
    struct timeval _startTime, _stopTime;
};


inline std::ostream &operator<<(std::ostream &os, const Timer &t) {
    return t.printOn(os);
}

#endif
