//
//  Copyright (c) 1994, 1995, 2004, 2006, 2015
//  Initial port performed by Stefan Eilemann (eilemann@gmail.com)
//
//  This file may be distributed under terms of the GPL
//
#include "gfxmeter.h"
#include "xosview.h"

#include <iostream>

#include <stdlib.h>
#include <unistd.h>
#include <invent.h>


// GfxMeter display swapbuffers per second. max is base rate for one gfx pipe.

GfxMeter::GfxMeter(XOSView *parent, int max)
    : FieldMeterGraph(parent, 2, "GFX","SWAPBUF/S", 1, 1, 0 ) {
    lastalarmstate = -1;
    total_ = 0;
    _nPipes = nPipes();
    total_ = _nPipes * max;

    if ( total_== 0 )
        logFatal << "nPipes * max is 0." << std::endl;
}

GfxMeter::~GfxMeter(void) {
}

int GfxMeter::nPipes(void) {
    int _nPipes = 0;

    setinvent();
    for( inventory_t *inv = getinvent(); inv != NULL; inv = getinvent() )
        if ( inv->inv_class==INV_GRAPHICS )
            _nPipes++;

    return _nPipes;
}

void GfxMeter::checkResources(void) {
    FieldMeterGraph::checkResources();

    swapgfxcol_ = parent_->g().allocColor(parent_->getResource(
          "gfxSwapColor" ));
    warngfxcol_ = parent_->g().allocColor(parent_->getResource(
          "gfxWarnColor" ));
    critgfxcol_ = parent_->g().allocColor(parent_->getResource(
          "gfxCritColor" ));

    setfieldcolor(0, parent_->getResource( "gfxSwapColor" ));
    setfieldcolor(1, parent_->getResource("gfxIdleColor"));
    priority_ = util::stoi (parent_->getResource("gfxPriority"));
    useGraph_ = parent_->isResourceTrue("gfxGraph");
    dodecay_ = parent_->isResourceTrue( "gfxDecay" );
    setUsedFormat(parent_->getResource("gfxUsedFormat"));

    warnThreshold = util::stoi (parent_->getResource("gfxWarnThreshold"))
        * _nPipes;
    critThreshold = util::stoi (parent_->getResource("gfxCritThreshold"))
        * _nPipes;

    if (dodecay_) {
        //  Warning:  Since the gfxmeter changes scale occasionally, old
        //  decay values need to be rescaled.  However, if they are rescaled,
        //  they could go off the edge of the screen.  Thus, for now, to
        //  prevent this whole problem, the gfx meter can not be a decay
        //  meter.  The gfx is a decaying average kind of thing anyway,
        //  so having a decaying gfx average is redundant.
        logProblem << "The gfxmeter can not be configured as a decay\n"
                   << "  meter. See the source code (" <<__FILE__
                   << ") for further\n"
                   << "  details.\n";
        dodecay_ = 0;
    }
}

void GfxMeter::checkevent(void) {
    getgfxinfo();
    drawfields(parent_->g());
}

void GfxMeter::getgfxinfo(void) {
    SarMeter::GfxInfo *gi = SarMeter::Instance()->getGfxInfo();

    fields_[0] = (float)gi->swapBuf;

    if ( fields_[0] < warnThreshold )
        alarmstate = 0;
    else
        if ( fields_[0] >= critThreshold )
            alarmstate = 2;
        else
            alarmstate = 1;

    if ( alarmstate != lastalarmstate ) {
        if ( alarmstate == 0 )
            setfieldcolor( 0, swapgfxcol_ );
        else
            if ( alarmstate == 1 )
                setfieldcolor( 0, warngfxcol_ );
            else
                setfieldcolor( 0, critgfxcol_ );
        if (dolegends())
            drawLegend(parent_->g());
        lastalarmstate = alarmstate;
    }

    if ( fields_[0]*5.0<total_ )
        total_ = fields_[0];
    else
        if ( fields_[0]>total_ )
            total_ = fields_[0]*5.0;

    if ( total_ < 1.0)
        total_ = 1.0;

    fields_[1] = (float) (total_ - fields_[0]);

    setUsed(fields_[0], (float) 1.0);
}
