//
//  Copyright (c) 1994, 1995, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//  2007 by Samuel Thibault ( samuel.thibault@ens-lyon.org )
//
//  This file may be distributed under terms of the GPL
//
#include "swapmeter.h"

#include <fstream>
#include <sstream>

extern "C" {
#include <mach.h>
#include <mach/mach_traps.h>
#include <mach/default_pager.h>
#include "get_def_pager.h"
}

SwapMeter::SwapMeter( XOSView *parent )
    : FieldMeterGraph( parent, 2, "SWAP", "ACTIVE/USED/FREE" ){
    def_pager = MACH_PORT_NULL;
}

SwapMeter::~SwapMeter( void ){
}

void SwapMeter::checkResources(const ResDB &rdb){
    FieldMeterGraph::checkResources(rdb);

    setfieldcolor( 0, rdb.getResource( "swapUsedColor" ) );
    setfieldcolor( 1, rdb.getResource( "swapFreeColor" ) );
    priority_ = util::stoi (rdb.getResource( "swapPriority" ) );
    dodecay_ = rdb.isResourceTrue( "swapDecay" );
    useGraph_ = rdb.isResourceTrue( "swapGraph" );
    setUsedFormat (rdb.getResource("swapUsedFormat"));
}

void SwapMeter::checkevent( void ){
    getswapinfo();
    drawfields(parent_->g());
}


void SwapMeter::getswapinfo( void ){
    kern_return_t err;

    if (def_pager == MACH_PORT_NULL)
        def_pager = get_def_pager();

    if (!MACH_PORT_VALID (def_pager)) {
        def_pager = MACH_PORT_DEAD;
        logFatal << "MACH_PORT_VALID is false." << std::endl;
    }

    err = default_pager_info (def_pager, &def_pager_info);
    if (err)
        logFatal << "default_pager_info(): " << util::strerror(err) << std::endl;

    total_ = def_pager_info.dpi_total_space;
    fields_[1] = def_pager_info.dpi_free_space;
    fields_[0] = total_ - fields_[1];

    if (total_)
        setUsed (fields_[0], total_);
}
