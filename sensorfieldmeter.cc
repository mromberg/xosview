//
//  Copyright (c) 2014, 2015 by Tomi Tapper <tomi.o.tapper@jyu.fi>
//
//  This file may be distributed under terms of the GPL
//
//  Put code common to *BSD and Linux sensor meters here.
//

#include "sensorfieldmeter.h"

#include <iomanip>
#include <cmath>


SensorFieldMeter::SensorFieldMeter( XOSView *parent, const std::string &title,
  const std::string &legend,  bool docaptions,
  bool dolegends, bool dousedlegends )
    : FieldMeter(parent, 3, title, legend, docaptions, dolegends, dousedlegends),
      high_(0), low_(0), has_high_(false), has_low_(false), negative_(false),
      actcolor_(0), highcolor_(0), lowcolor_(0) {

    setMetric(true);
}

SensorFieldMeter::~SensorFieldMeter( void ){
}

void SensorFieldMeter::updateLegend( void ){
    std::string lscale, tscale;
    std::ostringstream l;
    double limit = ( negative_ ? low_ : high_ ), total;
    total = scaleValue(total_, tscale);
    if ( (!negative_ && has_high_) || (negative_ && has_low_) ) {
        if ( ( 0.1 <= fabs(total_) && fabs(total_) < 9.95 ) ||
          ( 0.1 <= fabs(limit) && fabs(limit) < 9.95 ) ) {
            if ( unit_.size() ) {
                l << std::setprecision(1)
                  << "ACT(" << unit_ << ")/" << limit << "/" << total_;
            }
            else
                l << std::setprecision(1) << "ACT/" << limit << "/" << total_;
        }
        else if ( ( 9.95 <= fabs(total_) && fabs(total_) < 10000 ) ||
          ( 9.95 <= fabs(limit) && fabs(limit) < 10000 ) ) {
            if ( unit_.size() ) {
                l << std::setprecision(0)
                  << "ACT(" << unit_ << ")/" << limit << "/" << total_;
            }
            else
                l << std::setprecision(0) << "ACT/" << limit << "/" << total_;
        }
        else {
            limit = scaleValue(limit, lscale);
            if ( unit_.size() ) {
                l << std::setprecision(0)
                  << "ACT(" << unit_ << ")/" << limit << lscale << "/"
                  << total << tscale;
            }
            else {
                l << std::setprecision(0)
                  << "ACT/" << limit << lscale << "/" << total << tscale;
            }
        }
    }
    else {
        if ( ( 0.1 <= fabs(total_) && fabs(total_) < 9.95 ) ||
          ( 0.1 <= fabs(limit) && fabs(limit) < 9.95 ) ) {
            if ( unit_.size() ) {
                l << std::setprecision(1)
                  << "ACT(" << unit_ << ")/" << ( negative_ ? "LOW" : "HIGH" )
                  << "/" << total_;
            }
            else {
                l << std::setprecision(1)
                  << "ACT/" << ( negative_ ? "LOW" : "HIGH" )
                  << "/" << total_;
            }
        }
        else if ( ( 9.95 <= fabs(total_) && fabs(total_) < 10000 ) ||
          ( 9.95 <= fabs(limit) && fabs(limit) < 10000 ) ) {
            if ( unit_.size() ) {
                l << std::setprecision(0)
                  << "ACT(" << unit_ << ")/" << ( negative_ ? "LOW" : "HIGH" )
                  << "/" << total_;
            }
            else {
                l << std::setprecision(0)
                  << "ACT/" << ( negative_ ? "LOW" : "HIGH" )
                  << "/" << total_;
            }
        }
        else {
            if ( unit_.size() ) {
                l << std::setprecision(0)
                  << "ACT(" << unit_ << ")/" << ( negative_ ? "LOW" : "HIGH" )
                  << "/" << total_ << tscale;
            }
            else {
                l << std::setprecision(0)
                  << "ACT/" << ( negative_ ? "LOW" : "HIGH" )
                  << "/" << total << tscale;
            }
        }
    }

    legend(l.str());
}

/* Check if meter needs to be flipped, or total/limits changed. */
/* Check also if alarm limit is reached. */
/* This is to be called after the values have been read. */
void SensorFieldMeter::checkFields( double low, double high ){
    // Most sensors stay at either positive or negative values. Consider the
    // actual value and alarm limits when deciding should the meter be showing
    // positive or negative scale.
    bool do_legend = false;

    if (negative_) {  // negative at previous run
        if (fields_[0] >= 0 || low > 0) { // flip to positive
            negative_ = false;
            total_ = fabs(total_);
            high_ = ( has_high_ ? high : total_ );
            low_ = ( has_low_ ? low : 0 );
            setfieldcolor( 2, highcolor_ );
            do_legend = true;
        }
        else {
            if (has_low_ && low != low_) {
                low_ = low;
                do_legend = true;
            }
            if (has_high_ && high != high_)
                high_ = high;
        }
    }
    else {  // positive at previous run
        // flip to negative if value and either limit is < 0
        if ( fields_[0] < 0 &&
          ( (!has_low_ && !has_high_) || low < 0 || high < 0 ) ) {
            negative_ = true;
            total_ = -fabs(total_);
            high_ = ( has_high_ ? high : 0 );
            low_ = ( has_low_ ? low : total_ );
            setfieldcolor( 2, lowcolor_ );
            do_legend = true;
        }
        else {
            if (has_high_ && high != high_) {
                high_ = high;
                do_legend = true;
            }
            if (has_low_ && low != low_)
                low_ = low;
        }
    }

    // change total if value or alarms won't fit
    double highest = fabs(high_);
    if ( fabs(fields_[0]) > highest )
        highest = fabs(fields_[0]);
    if ( fabs(low_) > highest )
        highest = fabs(low_);
    if ( highest > fabs(total_) ) {
        do_legend = true;
        int scale = floor(log10(highest));
        total_ = ceil((highest / pow(10, scale)) * 1.25) * pow(10, scale);
        if (negative_) {
            total_ = -fabs(total_);
            if (!has_low_)
                low_ = total_;
            if (!has_high_)
                high_ = 0;
        }
        else {
            if (!has_low_)
                low_ = 0;
            if (!has_high_)
                high_ = total_;
        }
    }
    if (do_legend)
        updateLegend();

    // check for alarms
    if (fields_[0] > high_) { // alarm: T > max
        if (colors_[0] != highcolor_) {
            setfieldcolor( 0, highcolor_ );
            do_legend = true;
        }
    }
    else if (fields_[0] < low_) { // alarm: T < min
        if (colors_[0] != lowcolor_) {
            setfieldcolor( 0, lowcolor_ );
            do_legend = true;
        }
    }
    else {
        if (colors_[0] != actcolor_) {
            setfieldcolor( 0, actcolor_ );
            do_legend = true;
        }
    }

    setUsed(fields_[0], total_);
    if (negative_)
        fields_[1] = ( fields_[0] < low_ ? 0 : low_ - fields_[0] );
    else {
        if (fields_[0] < 0)
            fields_[0] = 0;
        fields_[1] = ( fields_[0] > high_ ? 0 : high_ - fields_[0] );
    }
    fields_[2] = total_ - fields_[1] - fields_[0];

    if (do_legend) {
        drawLegend(parent_->g());
    }
}
