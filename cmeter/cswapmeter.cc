//
//  Copyright (c) 1994, 1995, 2006, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "cswapmeter.h"



ComSwapMeter::ComSwapMeter( void )
: FieldMeterGraph( 2, "SWAP", "USED/FREE" ){
}


ComSwapMeter::~ComSwapMeter( void ){
}


void ComSwapMeter::checkResources(const ResDB &rdb){
    FieldMeterGraph::checkResources(rdb);

    setfieldcolor( 0, rdb.getColor( "swapUsedColor" ) );
    setfieldcolor( 1, rdb.getColor( "swapFreeColor" ) );
}


void ComSwapMeter::checkevent( void ){
    std::pair<uint64_t, uint64_t> swinfo = getswapinfo();
    _total = swinfo.first;
    _fields[0] = _total - swinfo.second;
    _fields[1] = swinfo.second;

    if (swinfo.first)
        setUsed (_fields[0], _total);
}
