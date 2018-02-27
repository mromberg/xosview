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

#ifndef memmeter_h
#define memmeter_h

#include "fieldmetergraph.h"



class MemMeter : public FieldMeterGraph {
public:
    MemMeter(void);

    virtual std::string resName(void) const override { return "mem"; }
    virtual void checkevent(void) override;
    virtual void checkResources(const ResDB &rdb) override;

private:
    std::vector<uint64_t> _meminfo;

    void getmeminfo(void);

    // layout: { active, inactive, wired, cached, free }
    void getMemStats(std::vector<uint64_t> &meminfo) const;
};


#endif
