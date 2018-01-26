//
//  Copyright (c) 1994, 1995, 2002, 2006, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "cnetmeter.h"



ComNetMeter::ComNetMeter( void )
    : FieldMeterGraph( 3, "NET", "IN/OUT/IDLE" ), _maxBandwidth(0) {
}


ComNetMeter::~ComNetMeter( void ){
}


void ComNetMeter::checkResources(const ResDB &rdb){
    FieldMeterGraph::checkResources(rdb);

    _maxBandwidth = std::stof(rdb.getResource( "netBandwidth" ));
    setfieldcolor( 0, rdb.getColor( "netInColor" ) );
    setfieldcolor( 1, rdb.getColor( "netOutColor" ) );
    setfieldcolor( 2, rdb.getColor( "netBackground" ) );
}


void ComNetMeter::checkevent(void) {

    std::pair<float, float> rates(getRates());

    if ((rates.first + rates.second) > _maxBandwidth) { // display percentages
        _total = rates.first + rates.second;
        _fields[0] = rates.first / _total;
        _fields[1] = rates.second / _total;
        _fields[2] = 0;
        _total = 1.0;
    }
    else {
        _total = _maxBandwidth;
        _fields[0] = rates.first;
        _fields[1] = rates.second;
        _fields[2] = _total - _fields[0] - _fields[1];
    }

    setUsed(rates.first + rates.second, 1.0);
}
