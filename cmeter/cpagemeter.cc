//
//  Copyright (c) 1996, 2007, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "cpagemeter.h"




ComPageMeter::ComPageMeter( XOSView *parent)
    : FieldMeterGraph( parent, 3, "PAGE", "IN/OUT/IDLE" ), _maxspeed(0) {
}


ComPageMeter::~ComPageMeter( void ){
}


void ComPageMeter::checkResources(const ResDB &rdb){
    FieldMeterGraph::checkResources(rdb);

    _maxspeed = util::stof(rdb.getResource("pageBandwidth"));
    setfieldcolor( 0, rdb.getColor( "pageInColor" ) );
    setfieldcolor( 1, rdb.getColor( "pageOutColor" ) );
    setfieldcolor( 2, rdb.getColor( "pageIdleColor" ) );
    priority_ = util::stoi (rdb.getResource( "pagePriority" ));
    setUsedFormat (rdb.getResource("pageUsedFormat"));
    decayUsed(rdb.isResourceTrue("pageUsedDecay"));
}


void ComPageMeter::checkevent( void ){

    std::pair<float, float> rates(getPageRate());

    setUsed(rates.first + rates.second, 1.0);

    if (rates.first + rates.second > _maxspeed) {
        total_ = rates.first + rates.second;
        fields_[0] = rates.first / total_;
        fields_[1] = rates.second / total_;
        fields_[2] = 0;
        total_ = 1.0;
    }
    else {
        total_ = _maxspeed;
        fields_[0] = rates.first;
        fields_[1] = rates.second;
        fields_[2] = total_ - fields_[0] - fields_[1];
    }
}
