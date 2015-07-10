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
// $Id: MeterMaker.h,v 1.6 2002/03/22 03:23:40 bgrayson Exp $
//
#ifndef _MeterMaker_h
#define _MeterMaker_h


#include "pllist.h"

#include <string>


class Meter;
class XOSView;

class MeterMaker : public PLList<Meter *> {
public:
    MeterMaker(XOSView *xos);
    void makeMeters(void);
private:
    XOSView *_xos;

    void cpuFactory(void);
    void getRange(const std::string &resource, size_t cpuCount,
      size_t &start, size_t &end) const;
};

#endif
