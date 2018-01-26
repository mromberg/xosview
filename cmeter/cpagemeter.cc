//
//  Copyright (c) 1996, 2007, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "cpagemeter.h"




ComPageMeter::ComPageMeter( void )
    : FieldMeterGraph( 3, "PAGE", "IN/OUT/IDLE" ), _maxspeed(0) {
}


ComPageMeter::~ComPageMeter( void ){
}


void ComPageMeter::checkResources(const ResDB &rdb){
    FieldMeterGraph::checkResources(rdb);

    _maxspeed = std::stof(rdb.getResource("pageBandwidth"));
    setfieldcolor( 0, rdb.getColor( "pageInColor" ) );
    setfieldcolor( 1, rdb.getColor( "pageOutColor" ) );
    setfieldcolor( 2, rdb.getColor( "pageIdleColor" ) );
}


void ComPageMeter::checkevent( void ){

    std::pair<float, float> rates(getPageRate());

    setUsed(rates.first + rates.second, 1.0);

    if (rates.first + rates.second > _maxspeed) {
        _total = rates.first + rates.second;
        _fields[0] = rates.first / _total;
        _fields[1] = rates.second / _total;
        _fields[2] = 0;
        _total = 1.0;
    }
    else {
        _total = _maxspeed;
        _fields[0] = rates.first;
        _fields[1] = rates.second;
        _fields[2] = _total - _fields[0] - _fields[1];
    }
}
