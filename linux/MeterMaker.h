//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef _MeterMaker_h
#define _MeterMaker_h

#include "pllist.h"
#include "rdb.h"

#include <string>


class Meter;
class XOSView;

class MeterMaker : public PLList<Meter *> {
public:
    MeterMaker(XOSView *xos);

    void makeMeters(const ResDB &rdb);

private:
    XOSView *_xos;

    void cpuFactory(const ResDB &rdb);
    void serialFactory(const ResDB &rdb);
    void intFactory(const ResDB &rdb);
    void tzoneFactory(void);
    void lmsTempFactory(const ResDB &rdb);
    void getRange(const ResDB &rdb, const std::string &resource,
      size_t cpuCount, size_t &start, size_t &end) const;
};

#endif
