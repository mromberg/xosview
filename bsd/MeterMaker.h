//
//  Copyright (c) 1994, 1995, 2015 by Mike Romberg ( romberg@fsl.noaa.gov )
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

#include "rdb.h"

#include <vector>


class Meter;



class MeterMaker {
public:
    MeterMaker(void);

    std::vector<Meter *> makeMeters(const ResDB &rdb);

private:
    std::vector<Meter *> _meters;

    void cpuFactory(const ResDB &rdb);
    void coreTempFactory(const ResDB &rdb);
    void sensorFactory(const ResDB &rdb);
};


#endif
