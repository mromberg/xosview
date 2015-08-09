//
//  Copyright (c) 1994, 1995, 1997, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "loadmeter.h"
#include "xosview.h"

#include <stdlib.h>
#include <sys/pstat.h>

LoadMeter::LoadMeter( XOSView *parent )
    : FieldMeterDecay( parent, 2, "LOAD", "PROCS/MIN", 1, 0 ){
}

LoadMeter::~LoadMeter( void ){
}

void LoadMeter::checkResources( void ){
    FieldMeterDecay::checkResources();

    warnloadcol_ = parent_->g().allocColor(parent_->getResource(
          "loadWarnColor" ));
    procloadcol_ = parent_->g().allocColor(parent_->getResource(
          "loadProcColor" ));

    setfieldcolor( 0, procloadcol_ );
    setfieldcolor( 1, parent_->getResource( "loadIdleColor" ) );
    priority_ = util::stoi (parent_->getResource( "loadPriority" ) );
    dodecay_ = parent_->isResourceTrue( "loadDecay" );
    setUsedFormat( parent_->getResource( "loadUsedFormat" ));

    alarmThreshold = util::stoi (parent_->getResource("loadAlarmThreshold"));

    if (dodecay_){
        //  Warning:  Since the loadmeter changes scale occasionally, old
        //  decay values need to be rescaled.  However, if they are rescaled,
        //  they could go off the edge of the screen.  Thus, for now, to
        //  prevent this whole problem, the load meter can not be a decay
        //  meter.  The load is a decaying average kind of thing anyway,
        //  so having a decaying load average is redundant.
        logProblem << "The loadmeter can not be configured as a decay\n"
                   << "  meter.  See the source code ("
                   << __FILE__ << ") for further\n"
                   << "  details.\n";
        dodecay_ = 0;
    }
}

void LoadMeter::checkevent( void ){
    getloadinfo();

    drawfields(parent_->g());
}


void LoadMeter::getloadinfo( void ){

    struct pst_dynamic pstd;

    pstat_getdynamic(&pstd, sizeof(pstd), 1, 0);

    fields_[0] = pstd.psd_avg_1_min;

    if ( fields_[0] > alarmThreshold ) {
        if (total_ == alarmThreshold ) {
            setfieldcolor( 0, warnloadcol_ );
            if (dolegends()) drawlegend(parent_->g());
        }
        total_ = fields_[1] = 20;
    } else {
        if (total_ == 20 ) {
            setfieldcolor( 0, procloadcol_ );
            if (dolegends()) drawLegend(parent_->g());
        }
        total_ = fields_[1] = alarmThreshold;
    }

    setUsed( fields_[0], total_ );
}
