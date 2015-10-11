//
//  Copyright (c) 1999, 2006, 2015
//  by Mike Romberg (mike-romberg@comcast.net)
//
//  This file may be distributed under terms of the GPL
//

#include "cdiskmeter.h"



ComDiskMeter::ComDiskMeter( XOSView *parent)
    : FieldMeterGraph(parent, 3, "DISK", "READ/WRITE/IDLE") {
}


ComDiskMeter::~ComDiskMeter( void ){
}


void ComDiskMeter::checkResources(const ResDB &rdb) {
    FieldMeterGraph::checkResources(rdb);

    maxspeed_ = util::stof(rdb.getResource("diskBandwidth"));
    setfieldcolor( 0, rdb.getColor("diskReadColor") );
    setfieldcolor( 1, rdb.getColor("diskWriteColor") );
    setfieldcolor( 2, rdb.getColor("diskIdleColor") );
    priority_ = util::stoi (rdb.getResource( "diskPriority" ));
}


void ComDiskMeter::checkevent( void ) {
    std::pair<double, double> rates(getRate());

    setUsed(rates.first + rates.second, 1.0);

    if (rates.first + rates.second > maxspeed_) {
        total_ = rates.first + rates.second;
        fields_[0] = rates.first / total_;
        fields_[1] = rates.second / total_;
        fields_[2] = 0;
        total_ = 1.0;
    }
    else {
        total_ = maxspeed_;
        fields_[0] = rates.first;
        fields_[1] = rates.second;
        fields_[2] = total_ - fields_[0] - fields_[1];
    }
}
