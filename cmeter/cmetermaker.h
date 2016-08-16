//
//  Copyright (c) 2016
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef cmetermaker_h
#define cmetermaker_h

#include <vector>
#include <string>

class Meter;


class ComMeterMaker {
protected:
    std::vector<Meter *> _meters;

    void getRange(const std::string &format, size_t cpuCount,
      size_t &start, size_t &end) const;
};

#endif
