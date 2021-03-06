//
//  Copyright (c) 1994, 1995, 2015, 2018
//  by Mike Romberg ( romberg@fsl.noaa.gov )
//  Copyright (c) 1995, 1996, 1997-2002 by Brian Grayson (bgrayson@netbsd.org)
//
//  Most of this code was written by Werner Fink <werner@suse.de>
//  Only small changes were made on my part (M.R.)
//  And the near-trivial port to NetBSD was by bgrayson.
//
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//

#ifndef loadmeter_h
#define loadmeter_h

#include "cloadmeter.h"



class LoadMeter : public ComLoadMeter {
public:
    LoadMeter(void);

protected:
    virtual float getLoad(void) override;
    virtual uint64_t getCPUSpeed(void) override;
};


#endif
