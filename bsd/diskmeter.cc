//
//  Copyright (c) 1995, 1996, 1997-2002, 2015
//  by Brian Grayson (bgrayson@netbsd.org)
//
//  This file was written by Brian Grayson for the NetBSD and xosview
//    projects.
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//

#include "diskmeter.h"
#include "kernel.h"


DiskMeter::DiskMeter( XOSView *parent, double max )
    : FieldMeterGraph( parent, 3, "DISK", "READ/WRITE/IDLE" ),
#ifndef HAVE_DEVSTAT
      prevreads_(0), prevwrites_(0),
#endif
      maxBandwidth_(max) {

    dodecay_ = 0;
    total_ = max;
    if (!BSDDiskInit())
        disableMeter();

    /* Since at the first call, it will look like we transferred a
     * gazillion bytes, let's reset total_ again and do another
     * call.  This will force total_ to be something reasonable.  */
    getstats();
    total_ = max;
    getstats();
    IntervalTimerStart();
    /* By doing this check, we eliminate the startup situation where
     * all fields are 0, and total is 0, leading to nothing being drawn
     * on the meter.  So, make it look like nothing was transferred,
     * out of a total of 1 bytes.  */
    fields_[0] = fields_[1] = 0.0;
    total_ = 1.0;
    fields_[2] = total_;
}


DiskMeter::~DiskMeter( void ) {
}


void DiskMeter::checkResources(const ResDB &rdb) {
	FieldMeterGraph::checkResources(rdb);

	setfieldcolor( 0, rdb.getColor("diskReadColor") );
	setfieldcolor( 1, rdb.getColor("diskWriteColor") );
	setfieldcolor( 2, rdb.getColor("diskIdleColor") );
	priority_ = util::stoi( rdb.getResource("diskPriority") );
	dodecay_ = rdb.isResourceTrue("diskDecay");
	useGraph_ = rdb.isResourceTrue("diskGraph");
	setUsedFormat( rdb.getResource("diskUsedFormat") );
}


void DiskMeter::checkevent( void ) {
	getstats();
	drawfields(parent_->g());
}


void DiskMeter::getstats( void ) {
	uint64_t reads = 0, writes = 0;
	//  Reset to desired full-scale settings.
	total_ = maxBandwidth_;

	IntervalTimerStop();
	BSDGetDiskXFerBytes(reads, writes);

	/*  Adjust this to bytes/second.  */
#if defined(HAVE_DEVSTAT)
	fields_[0] = reads / IntervalTimeInSecs();
	fields_[1] = writes / IntervalTimeInSecs();
#else
	fields_[0] = (reads - prevreads_) / IntervalTimeInSecs();
	fields_[1] = (writes - prevwrites_) / IntervalTimeInSecs();
	prevreads_ = reads;
	prevwrites_ = writes;
#endif
	IntervalTimerStart();

	/*  Adjust in case of first call.  */
	if (fields_[0] < 0.0)
		fields_[0] = 0.0;
	if (fields_[1] < 0.0)
		fields_[1] = 0.0;

	/*  Adjust total_ if needed.  */
	if (fields_[0] + fields_[1] > total_)
		total_ = fields_[0] + fields_[1];

	fields_[2] = total_ - (fields_[0] + fields_[1]);
        for (size_t i = 0 ; i < 3 ; i++)
            if (fields_[i] < 0.0) {
                logDebug << "diskmeter: fields[" << i << "] of "
                         << fields_[i] << " is < 0" << std::endl;
            }

	setUsed(fields_[0] + fields_[1], total_);
}
