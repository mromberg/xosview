//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "cswapmeter.h"



ComSwapMeter::ComSwapMeter( XOSView *parent )
: FieldMeterGraph( parent, 2, "SWAP", "USED/FREE" ){
}


ComSwapMeter::~ComSwapMeter( void ){
}


void ComSwapMeter::checkResources(const ResDB &rdb){
    FieldMeterGraph::checkResources(rdb);

    setfieldcolor( 0, rdb.getColor( "swapUsedColor" ) );
    setfieldcolor( 1, rdb.getColor( "swapFreeColor" ) );
    priority_ = util::stoi (rdb.getResource( "swapPriority" ));
}


void ComSwapMeter::checkevent( void ){
    std::pair<uint64_t, uint64_t> swinfo = getswapinfo();
    total_ = swinfo.first;
    fields_[0] = total_ - swinfo.second;
    fields_[1] = swinfo.second;

    if (swinfo.first)
        setUsed (fields_[0], total_);
}
