//
//  Copyright (c) 1994, 1995, 2015, 2016, 2018
//  by Mike Romberg ( romberg@fsl.noaa.gov )
//  Copyright (c) 1995, 1996, 1997-2002 by Brian Grayson (bgrayson@netbsd.org)
//
//  This file was written by Brian Grayson for the NetBSD and xosview
//    projects.
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//

#ifndef METERMAKER_H
#define METERMAKER_H

#include "cmetermaker.h"

class ResDB;


class MeterMaker : public ComMeterMaker {
public:
    mlist makeMeters(const ResDB &rdb);

private:
    void cpuFactory(const ResDB &rdb, mlist &meters) const;
    void coreTempFactory(const ResDB &rdb, mlist &meters) const;
    void sensorFactory(const ResDB &rdb, mlist &meters) const;
};

#endif
