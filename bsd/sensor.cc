//
//  Copyright (c) 2012, 2015 by Tomi Tapper <tomi.o.tapper@jyu.fi>
//
//  File based on linux/lmstemp.* by
//  Copyright (c) 2000, 2006 by Leopold Toetsch <lt@toetsch.at>
//
//  This file may be distributed under terms of the GPL
//
//
//

#include "sensor.h"
#include "kernel.h"

#include <cmath>


BSDSensor::BSDSensor( XOSView *parent, const std::string &name,
  const std::string &high,
  const std::string &low, const std::string &label,
  const std::string &caption, int nbr )
    : SensorFieldMeter( parent, label, caption, 1, 1, 0 ) {

    nbr_ = nbr;
    highname_[0] = highval_[0] = '\0';
    lowname_[0] = lowval_[0] = '\0';
    std::string n(name);
    name_ = n.substr( 0, n.find_first_of('.') );
    val_ = n.substr( n.find_first_of('.') + 1 );
    if (high.size()) {
        has_high_ = true;
        if (!util::fstr(high, high_)) { // high given as number?
            n = high;
            highname_ = n.substr( 0, n.find_first_of('.') );
            highval_ = n.substr( n.find_first_of('.') + 1 );
        }
    }
    if (low.size()) {
        has_low_ = true;
        if (!util::fstr(low, low_)) {  // low given as number?
            n = low;
            lowname_ = n.substr( 0, n.find_first_of('.') );
            lowval_ = n.substr( n.find_first_of('.') + 1 );
        }
    }
}


BSDSensor::~BSDSensor( void ) {
}

void BSDSensor::checkResources( void ) {
    SensorFieldMeter::checkResources();

    actcolor_  = parent_->g().allocColor( parent_->getResource(
          "bsdsensorActColor" ) );
    highcolor_ = parent_->g().allocColor( parent_->getResource(
          "bsdsensorHighColor" ) );
    lowcolor_  = parent_->g().allocColor( parent_->getResource(
          "bsdsensorLowColor" ) );
    setfieldcolor( 0, actcolor_  );
    setfieldcolor( 1, parent_->getResource( "bsdsensorIdleColor" ) );
    setfieldcolor( 2, highcolor_ );
    priority_ = util::stoi( parent_->getResource( "bsdsensorPriority" ) );

    std::string tmp(parent_->getResourceOrUseDefault(
          "bsdsensorHighest", "0" ));
    std::string s("bsdsensorHighest");
    s += util::repr(nbr_);
    total_ = fabs( util::stof( parent_->getResourceOrUseDefault(s, tmp) ) );
    s = "bsdsensorUsedFormat" + util::repr(nbr_);
    std::string f = parent_->getResourceOrUseDefault(s, "");
    setUsedFormat( f.size() ? f : parent_->getResource(
          "bsdsensorUsedFormat" ) );

    if (!has_high_)
        high_ = total_;
    if (!has_low_)
        low_ = 0;

    // Get the unit.
    float dummy;
    BSDGetSensor(name_.c_str(), val_.c_str(), &dummy, unit_);
    updateLegend();
}

void BSDSensor::checkevent( void ) {
    getsensor();
    drawfields(parent_->g());
}

void BSDSensor::getsensor( void ) {
    float value, high = high_, low = low_;
    std::string emptyStr;
    BSDGetSensor(name_.c_str(), val_.c_str(), &value, emptyStr);
    if ( highname_.size() )
        BSDGetSensor(highname_.c_str(), highval_.c_str(), &high, emptyStr);
    if ( lowname_.size() )
        BSDGetSensor(lowname_.c_str(), lowval_.c_str(), &low, emptyStr);

    fields_[0] = value;
    checkFields(low, high);
}
