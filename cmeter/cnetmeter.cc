//
//  Copyright (c) 1994, 1995, 2002, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "cnetmeter.h"



ComNetMeter::ComNetMeter( XOSView *parent)
    : FieldMeterGraph( parent, 3, "NET", "IN/OUT/IDLE" ), _maxBandwidth(0) {
}


ComNetMeter::~ComNetMeter( void ){
}


void ComNetMeter::checkResources(const ResDB &rdb){
    FieldMeterGraph::checkResources(rdb);

    _maxBandwidth = util::stof(rdb.getResource( "netBandwidth" ));
    setfieldcolor( 0, rdb.getColor( "netInColor" ) );
    setfieldcolor( 1, rdb.getColor( "netOutColor" ) );
    setfieldcolor( 2, rdb.getColor( "netBackground" ) );
}


void ComNetMeter::checkevent(void) {

    std::pair<float, float> rates(getRates());

    if ((rates.first + rates.second) > _maxBandwidth) { // display percentages
        total_ = rates.first + rates.second;
        fields_[0] = rates.first / total_;
        fields_[1] = rates.second / total_;
        fields_[2] = 0;
        total_ = 1.0;
    }
    else {
        total_ = _maxBandwidth;
        fields_[0] = rates.first;
        fields_[1] = rates.second;
        fields_[2] = total_ - fields_[0] - fields_[1];
    }

    setUsed(rates.first + rates.second, 1.0);
}
