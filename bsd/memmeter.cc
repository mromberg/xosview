//
//  Copyright (c) 1994, 1995, 2015
//  by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  NetBSD port:
//  Copyright (c) 1995, 1996, 1997-2002 by Brian Grayson (bgrayson@netbsd.org)
//
//  This file was originally written by Brian Grayson for the NetBSD and
//    xosview projects.
//  The NetBSD memmeter was improved by Tom Pavel (pavel@slac.stanford.edu)
//    to provide active and inactive values, rather than just "used."
//
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//

#include "memmeter.h"
#include "kernel.h"



#if defined(HAVE_UVM)
static const char * const LEGEND = "ACT/INACT/WRD/FREE";
static size_t NFIELDS = 4;
#else
static const char * const LEGEND = "ACT/INACT/WRD/CA/FREE";
static size_t NFIELDS = 5;
#endif



MemMeter::MemMeter( XOSView *parent )
    : FieldMeterGraph( parent, NFIELDS, "MEM", LEGEND ),
      meminfo_(5, 0) {

    BSDPageInit();
}


MemMeter::~MemMeter( void ) {
}


void MemMeter::checkResources(const ResDB &rdb) {

    FieldMeterGraph::checkResources(rdb);

    setfieldcolor( 0, rdb.getColor("memActiveColor") );
    setfieldcolor( 1, rdb.getColor("memInactiveColor") );
    setfieldcolor( 2, rdb.getColor("memWiredColor") );
#if defined(HAVE_UVM)
    setfieldcolor( 3, rdb.getColor("memFreeColor") );
#else
    setfieldcolor( 3, rdb.getColor("memCacheColor") );
    setfieldcolor( 4, rdb.getColor("memFreeColor") );
#endif
}


void MemMeter::checkevent( void ) {
    getmeminfo();
}


void MemMeter::getmeminfo( void ) {
    BSDGetMemStats(meminfo_);
    fields_[0] = (double)meminfo_[0];
    fields_[1] = (double)meminfo_[1];
    fields_[2] = (double)meminfo_[2];
#if defined(HAVE_UVM)
    fields_[3] = (double)meminfo_[4];
#else
    fields_[3] = (double)meminfo_[3];
    fields_[4] = (double)meminfo_[4];
#endif
    total_ = (double)(meminfo_[0] + meminfo_[1] + meminfo_[2] + meminfo_[3]
      + meminfo_[4]);
    setUsed(total_ - (double)meminfo_[4], total_);
}
