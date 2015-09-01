//
//  Copyright (c) 1999, 2006, 2015
//  by Mike Romberg (mike-romberg@comcast.net)
//
//  This file may be distributed under terms of the GPL
//
#include "diskmeter.h"




DiskMeter::DiskMeter( XOSView *parent)
    : FieldMeterGraph(parent, 3, "DISK", "READ/WRITE/IDLE"), _max(0) {

    if (!_query.add("\\LogicalDisk(_Total)\\Disk Read Bytes/sec")
      || !_query.add("\\LogicalDisk(_Total)\\Disk Write Bytes/sec"))
        logFatal << "failed to load counters." << std::endl;

    _query.query();
}


DiskMeter::~DiskMeter( void ){
}


void DiskMeter::checkResources(const ResDB &rdb) {
    FieldMeterGraph::checkResources(rdb);

    setfieldcolor( 0, rdb.getColor("diskReadColor") );
    setfieldcolor( 1, rdb.getColor("diskWriteColor") );
    setfieldcolor( 2, rdb.getColor("diskIdleColor") );
    priority_ = util::stoi (rdb.getResource( "diskPriority" ));
    _max = util::stof(rdb.getResource("diskBandwidth"));
    dodecay_ = rdb.isResourceTrue("diskDecay" );
    useGraph_ = rdb.isResourceTrue( "diskGraph" );
    setUsedFormat(rdb.getResource("diskUsedFormat"));
    decayUsed(rdb.isResourceTrue("diskUsedDecay"));
}


void DiskMeter::checkevent( void ) {
    _query.query();

    double read = _query.counters()[0].doubleVal();
    double write = _query.counters()[1].doubleVal();

    if ((read + write) > _max) { // display percentages
        total_ = (read + write);
        fields_[0] = read / total_;
        fields_[1] = write / total_;
        fields_[2] = 0;
        total_ = 1.0;
    }
    else {
        total_ = _max;
        fields_[0] = read;
        fields_[1] = write;
        fields_[2] = total_ - fields_[0] - fields_[1];
    }

    setUsed(read + write, _max);
}
