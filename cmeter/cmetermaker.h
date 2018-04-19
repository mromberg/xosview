//
//  Copyright (c) 2016, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef cmetermaker_h
#define cmetermaker_h

#include <vector>
#include <string>
#include <memory>

class Meter;



class ComMeterMaker {
public:
    using mlist = std::vector<std::unique_ptr<Meter>>;

protected:
    void getRange(const std::string &format, size_t cpuCount,
      size_t &start, size_t &end) const;
};

#endif
