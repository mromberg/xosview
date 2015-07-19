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
    std::string n(name), tmp;
    tmp = n.substr( 0, n.find_first_of('.') );
    strncpy(name_, tmp.c_str(), NAMESIZE);
    tmp = n.substr( n.find_first_of('.') + 1 );
    strncpy(val_, tmp.c_str(), NAMESIZE);
    if (high.size()) {
        has_high_ = true;
        if (!util::fstr(high, high_)) { // high given as number?
            n = high;
            tmp = n.substr( 0, n.find_first_of('.') );
            strncpy(highname_, tmp.c_str(), NAMESIZE);
            tmp = n.substr( n.find_first_of('.') + 1 );
            strncpy(highval_, tmp.c_str(), NAMESIZE);
        }
    }
    if (low.size()) {
        has_low_ = true;
        if (!util::fstr(low, low_)) {  // low given as number?
            n = low;
            tmp = n.substr( 0, n.find_first_of('.') );
            strncpy(lowname_, tmp.c_str(), NAMESIZE);
            tmp = n.substr( n.find_first_of('.') + 1 );
            strncpy(lowval_, tmp.c_str(), NAMESIZE);
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
    BSDGetSensor(name_, val_, &dummy, unit_);
    updateLegend();
}

void BSDSensor::checkevent( void ) {
    getsensor();
    drawfields(parent_->g());
}

void BSDSensor::getsensor( void ) {
    float value, high = high_, low = low_;
    std::string emptyStr;
    BSDGetSensor(name_, val_, &value, emptyStr);
    if ( strlen(highname_) )
        BSDGetSensor(highname_, highval_, &high, emptyStr);
    if ( strlen(lowname_) )
        BSDGetSensor(lowname_, lowval_, &low, emptyStr);

    fields_[0] = value;
    checkFields(low, high);
}
