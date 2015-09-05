//
//  Copyright (c) 1994, 1995, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//  2007 by Samuel Thibault ( samuel.thibault@ens-lyon.org )
//
//  This file may be distributed under terms of the GPL
//
#ifndef METERMAKER_H
#define METERMAKER_H

#include "rdb.h"

#include <vector>


class Meter;
class XOSView;

class MeterMaker {
public:
    MeterMaker(XOSView *xos);

    std::vector<Meter *> makeMeters(const ResDB &rdb);

private:
    XOSView *_xos;
    std::vector<Meter *> _meters;

    void cpuFactory(const ResDB &rdb);
    void getRange(const ResDB &rdb, const std::string &resource,
      size_t cpuCount, size_t &start, size_t &end) const;
};

#endif
