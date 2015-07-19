//
//  Copyright (c) 2013, 2015 by Tomi Tapper ( tomi.o.tapper@student.jyu.fi )
//
//  Based on linux/btrymeter.cc:
//  Copyright (c) 1997, 2006 by Mike Romberg ( mike.romberg@noaa.gov )
//
//  This file may be distributed under terms of the GPL
//
#include "btrymeter.h"

#include "kernel.h"


BtryMeter::BtryMeter( XOSView *parent )
    : FieldMeter( parent, 2, "BTRY", "CHRG/USED", 1, 1, 0 ),
      leftcolor_(0), usedcolor_(0), chargecolor_(0), fullcolor_(0),
      lowcolor_(0), critcolor_(0), nonecolor_(0),
      old_state_(256) {
}

BtryMeter::~BtryMeter( void ) {
}

void BtryMeter::checkResources( void ) {
    FieldMeter::checkResources();

    leftcolor_ = parent_->g().allocColor( parent_->getResource(
          "batteryLeftColor") );
    usedcolor_ = parent_->g().allocColor( parent_->getResource(
          "batteryUsedColor") );
    chargecolor_ = parent_->g().allocColor( parent_->getResource(
          "batteryChargeColor") );
    fullcolor_ = parent_->g().allocColor( parent_->getResource(
          "batteryFullColor") );
    lowcolor_ = parent_->g().allocColor( parent_->getResource(
          "batteryLowColor") );
    critcolor_ = parent_->g().allocColor( parent_->getResource(
          "batteryCritColor") );
    nonecolor_ = parent_->g().allocColor( parent_->getResource(
          "batteryNoneColor") );

    setfieldcolor(0, leftcolor_);
    setfieldcolor(1, usedcolor_);

    priority_ = util::stoi( parent_->getResource("batteryPriority") );
    setUsedFormat( parent_->getResource("batteryUsedFormat") );
}

void BtryMeter::checkevent( void ) {
    getstats();
    drawfields(parent_->g());
}

void BtryMeter::getstats( void ) {
    int remaining;
    unsigned int state;

    BSDGetBatteryInfo(&remaining, &state);

    if (state != old_state_) {
        if (state == XOSVIEW_BATT_NONE) { // no battery present
            setfieldcolor(0, nonecolor_);
            legend("NONE/NONE");
        }
        else if (state & XOSVIEW_BATT_FULL) { // full battery
            setfieldcolor(0, fullcolor_);
            legend("CHRG/FULL");
        }
        else { // present, not full
            if (state & XOSVIEW_BATT_CRITICAL) // critical charge
                setfieldcolor(0, critcolor_);
            else if (state & XOSVIEW_BATT_LOW) // low charge
                setfieldcolor(0, lowcolor_);
            else { // above low, below full
                if (state & XOSVIEW_BATT_CHARGING) // is charging
                    setfieldcolor(0, chargecolor_);
                else
                    setfieldcolor(0, leftcolor_);
            }
            // legend tells if charging or discharging
            if (state & XOSVIEW_BATT_CHARGING)
                legend("CHRG/AC");
            else
                legend("CHRG/USED");
        }
        drawLegend(parent_->g());
        old_state_ = state;
    }

    total_ = 100.0;
    fields_[0] = remaining;
    fields_[1] = total_ - remaining;
    setUsed(fields_[0], total_);
}
