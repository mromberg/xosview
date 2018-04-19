//
//  Copyright (c) 1994, 1995, 2015, 2016, 2018
//  by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  NetBSD port:
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

#ifndef pagemeter_h
#define pagemeter_h

#include "cpagemeter.h"


class PageMeter : public ComPageMeter {
public:
    PageMeter(void);

protected:
    virtual std::pair<float, float> getPageRate(void) override;

private:
    const size_t _psize;
    std::vector<uint64_t> _previnfo;

    // layout: { pages_in, pages_out }
    void getPageStats(std::vector<uint64_t> &pageinfo) const;
};


#endif
