//
//  Copyright (c) 1994, 1995, 2015 by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  NetBSD port:
//  Copyright (c) 1995, 1996, 1997-2002 by Brian Grayson (bgrayson@netbsd.org)
//
//  This file was originally written by Brian Grayson for the NetBSD and
//    xosview projects.
//
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//

#include "pagemeter.h"
#include "kernel.h"



PageMeter::PageMeter( XOSView *parent, double total )
    : FieldMeterGraph( parent, 3, "PAGE", "IN/OUT/IDLE" ),
      previnfo_(2, 0) {

    total_ = total;
    BSDPageInit();
    BSDGetPageStats(previnfo_);
}


PageMeter::~PageMeter( void ) {
}


void PageMeter::checkResources(const ResDB &rdb) {

    FieldMeterGraph::checkResources(rdb);

    setfieldcolor( 0, rdb.getColor("pageInColor") );
    setfieldcolor( 1, rdb.getColor("pageOutColor") );
    setfieldcolor( 2, rdb.getColor("pageIdleColor") );
    priority_ = util::stoi( rdb.getResource("pagePriority") );
    dodecay_ = rdb.isResourceTrue("pageDecay");
    useGraph_ = rdb.isResourceTrue("pageGraph");
    setUsedFormat( rdb.getResource("pageUsedFormat") );
}


void PageMeter::checkevent( void ) {
    getpageinfo();
    drawfields(parent_->g());
}


void PageMeter::getpageinfo( void ) {
    std::vector<uint64_t> info;
    BSDGetPageStats(info);

    fields_[0] = info[0] - previnfo_[0];
    fields_[1] = info[1] - previnfo_[1];
    previnfo_[0] = info[0];
    previnfo_[1] = info[1];

    if (total_ < fields_[0] + fields_[1])
        total_ = fields_[0] + fields_[1];

    fields_[2] = total_ - fields_[0] - fields_[1];
    setUsed(total_ - fields_[2], total_);
}
