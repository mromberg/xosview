//
//  Copyright (c) 1994, 1995, 2015 by Mike Romberg ( romberg@fsl.noaa.gov )
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

#include "netmeter.h"
#include "kernel.h"
#include <stdlib.h>


NetMeter::NetMeter( XOSView *parent, double max )
    : FieldMeterGraph( parent, 3, "NET", "IN/OUT/IDLE" ),
      lastBytesIn_(0), lastBytesOut_(0),
      netBandwidth_(0),
      ignored_(false) {

    if (!BSDNetInit()) {
        logProblem << "The kernel does not seem to have the symbols needed "
                   << "for the NetMeter.\n"
                   << "The NetMeter has been disabled." << std::endl;
        disableMeter ();
    }
    else {
        total_ = netBandwidth_ = max;
        lastBytesIn_ = lastBytesOut_ = 0;
        netIface_ = "False";
        ignored_ = false;
        BSDGetNetInOut(&lastBytesIn_, &lastBytesOut_, netIface_.c_str(),
          ignored_);
        IntervalTimerStart();
    }
}


NetMeter::~NetMeter( void ) {
}


void NetMeter::checkResources(const ResDB &rdb) {

    FieldMeterGraph::checkResources(rdb);

    setfieldcolor( 0, rdb.getResource("netInColor") );
    setfieldcolor( 1, rdb.getResource("netOutColor") );
    setfieldcolor( 2, rdb.getResource("netBackground") );
    priority_ = util::stoi( rdb.getResource("netPriority") );
    dodecay_ = rdb.isResourceTrue("netDecay");
    useGraph_ = rdb.isResourceTrue("netGraph");
    setUsedFormat( rdb.getResource("netUsedFormat") );
    netIface_ = rdb.getResource("netIface");
    if (netIface_[0] == '-') {
        ignored_ = true;
        netIface_.erase(0, netIface_.find_first_not_of("- "));
    }
}


void NetMeter::checkevent( void ) {
    getstats();
    drawfields(parent_->g());
}


void NetMeter::getstats(void) {
    //  Reset total_ to expected maximum.  If it is too low, it
    //  will be adjusted in adjust().  bgrayson
    total_ = netBandwidth_;
    fields_[0] = fields_[1] = 0;
    uint64_t nowBytesIn, nowBytesOut;

    IntervalTimerStop();
    BSDGetNetInOut(&nowBytesIn, &nowBytesOut, netIface_.c_str(), ignored_);
    double t = (1.0) / IntervalTimeInSecs();
    IntervalTimerStart();

    fields_[0] = (double)(nowBytesIn - lastBytesIn_) * t;
    lastBytesIn_ = nowBytesIn;
    fields_[1] = (double)(nowBytesOut - lastBytesOut_) * t;
    lastBytesOut_ = nowBytesOut;

    if (total_ < (fields_[0] + fields_[1]))
        total_ = fields_[0] + fields_[1];
    fields_[2] = total_ - fields_[0] - fields_[1];
    /*  The fields_ values have already been scaled into bytes/sec by
     *  the manipulations (* t) above.  */
    setUsed(fields_[0] + fields_[1], total_);
}
