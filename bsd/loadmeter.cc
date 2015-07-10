//
//  Copyright (c) 1994, 1995, 2015 by Mike Romberg ( romberg@fsl.noaa.gov )
//  Copyright (c) 1995, 1996, 1997-2002 by Brian Grayson (bgrayson@netbsd.org)
//
//  Most of this code was written by Werner Fink <werner@suse.de>.
//  Only small changes were made on my part (M.R.)
//  And the near-trivial port to NetBSD was done by Brian Grayson
//
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//
#include "loadmeter.h"

#include <stdlib.h>  //  for getloadavg()


LoadMeter::LoadMeter( XOSView *parent )
    : FieldMeterGraph( parent, 2, "LOAD", "PROCS per MIN/IDLE", 1, 1, 0 ),
      _procloadcol(0), _warnloadcol(0), _critloadcol(0), _warnThreshold(0),
      _critThreshold(0), _alarmstate(0), _lastalarmstate(0) {
}

LoadMeter::~LoadMeter( void ){
}

void LoadMeter::checkResources( void ){
    FieldMeterGraph::checkResources();

    _procloadcol = parent_->g().allocColor(
        parent_->getResource( "loadProcColor" ));
    _warnloadcol = parent_->g().allocColor(
        parent_->getResource( "loadWarnColor" ));
    _critloadcol = parent_->g().allocColor(
        parent_->getResource( "loadCritColor" ));

    setfieldcolor( 0, _procloadcol );
    setfieldcolor( 1, parent_->getResource( "loadIdleColor" ) );

    priority_ = util::stoi (parent_->getResource("loadPriority"));
    dodecay_ = parent_->isResourceTrue("loadDecay");
    useGraph_ = parent_->isResourceTrue("loadGraph");
    setUsedFormat (parent_->getResource("loadUsedFormat"));
    _warnThreshold = util::stoi (parent_->getResource("loadWarnThreshold"));
    _critThreshold = util::stoi (parent_->getResource("loadCritThreshold"));

    _alarmstate = _lastalarmstate = 0;

    if (dodecay_){
        //  Warning:  Since the loadmeter changes scale occasionally, old
        //  decay values need to be rescaled.  However, if they are rescaled,
        //  they could go off the edge of the screen.  Thus, for now, to
        //  prevent this whole problem, the load meter can not be a decay
        //  meter.  The load is a decaying average kind of thing anyway,
        //  so having a decaying load average is redundant.
        logProblem << "The loadmeter can not be configured as a decay\n"
                   << "  meter.  See the source code (" << __FILE__
                   << ") for further\n" << "  details.\n";
        dodecay_ = 0;
    }
}

void LoadMeter::checkevent( void ){
    getloadinfo();
    drawfields(parent_->g());
}

void LoadMeter::getloadinfo( void ){
    double oneMinLoad;

    getloadavg (&oneMinLoad, 1);  //  Only get the 1-minute-average sample.
    fields_[0] = oneMinLoad;  //  Convert from double to float.

    if ( fields_[0] <  _warnThreshold )
        _alarmstate = 0;
    else
        if ( fields_[0] >= _critThreshold )
            _alarmstate = 2;
        else
            /* if fields_[0] >= warnThreshold */ _alarmstate = 1;

    if (_alarmstate != _lastalarmstate) {
        if ( _alarmstate == 0 )
            setfieldcolor( 0, _procloadcol );
        else
            if ( _alarmstate == 1 )
                setfieldcolor( 0, _warnloadcol );
            else
                /* if alarmstate == 2 */ setfieldcolor( 0, _critloadcol );
        if (dolegends())
            drawLegend(parent_->g());
        _lastalarmstate = _alarmstate;
    }


    //  This method of auto-adjust is better than the old way.
    //  If fields[0] is less than 20% of display, shrink display to be
    //  full-width.  Then, if full-width < 1.0, set it to be 1.0.
    if ( fields_[0]*5.0<total_ )
        total_ = fields_[0];
    else
        //  If fields[0] is larger, then set it to be 1/5th of full.
        if ( fields_[0]>total_ )
            total_ = fields_[0]*5.0;

    if ( total_ < 1.0)
        total_ = 1.0;

    fields_[1] = (float) (total_ - fields_[0]);

    /*  I don't see why anyone would want to use any format besides
     *  float, but just in case.... */
    setUsed (fields_[0], total_);
}
