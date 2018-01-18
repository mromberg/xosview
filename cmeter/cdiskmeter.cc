//
//  Copyright (c) 1999, 2006, 2015, 2018
//  by Mike Romberg (mike-romberg@comcast.net)
//
//  This file may be distributed under terms of the GPL
//

#include "cdiskmeter.h"



ComDiskMeter::ComDiskMeter( void )
    : FieldMeterGraph(3, "DISK", "READ/WRITE/IDLE") {
}


ComDiskMeter::~ComDiskMeter( void ){
}


void ComDiskMeter::checkResources(const ResDB &rdb) {
    FieldMeterGraph::checkResources(rdb);

    maxspeed_ = util::stof(rdb.getResource("diskBandwidth"));
    setfieldcolor( 0, rdb.getColor("diskReadColor") );
    setfieldcolor( 1, rdb.getColor("diskWriteColor") );
    setfieldcolor( 2, rdb.getColor("diskIdleColor") );
}


void ComDiskMeter::checkevent( void ) {
    std::pair<double, double> rates(getRate());

    setUsed(rates.first + rates.second, 1.0);

    if (rates.first + rates.second > maxspeed_) {
        _total = rates.first + rates.second;
        _fields[0] = rates.first / _total;
        _fields[1] = rates.second / _total;
        _fields[2] = 0;
        _total = 1.0;
    }
    else {
        _total = maxspeed_;
        _fields[0] = rates.first;
        _fields[1] = rates.second;
        _fields[2] = _total - _fields[0] - _fields[1];
    }
}
