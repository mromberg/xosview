//
//  NetBSD port:
//  Copyright (c) 1995,1996,1997,2015,2016,2018
//  by Brian Grayson(bgrayson@netbsd.org)
//
//  This file was written by Brian Grayson for the NetBSD and xosview
//    projects.
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//

#ifndef diskmeter_h
#define diskmeter_h

#include "cdiskmeter.h"



class DiskMeter : public ComDiskMeter {
public:
    DiskMeter(void);

protected:
    virtual std::pair<double, double> getRate(void);

private:
    uint64_t prevreads_, prevwrites_;

    static void getDiskXFerBytes(uint64_t &rbytes,
      uint64_t &wbytes);
};


#endif
