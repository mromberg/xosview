//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "fieldmeter.h"
#include "xosview.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

// Size in sample to use for decaying used labels.
// Not scientific.  Just set to a value that caused
// the twitchy cpu meters to stop hammering the X server
// with text draws yet still give some reasonable
// update rate.
static const size_t DECAYN = 8;

FieldMeter::FieldMeter(XOSView *parent, size_t numfields,
  const std::string &title, const std::string &legend)
    : Meter(parent, title, legend),
      fields_(numfields, 0.0), total_(1.0),
      lastvals_(numfields, 0.0),
      lastx_(numfields, 0), used_(0), colors_(numfields, 0),
      print_(PERCENT), printedZeroTotalMesg_(0),
      _usedAvg(DECAYN, 0.0), _usedAvgIndex(0),
      _decayUsed(false), _used(0, 0, Label::BLSE) {
}


void FieldMeter::resize( int x, int y, int width, int height ) {
    Meter::resize(x, y, width, height);
    _used.move(Meter::x() - 2, Meter::y() + Meter::height() + 2);
}


FieldMeter::~FieldMeter( void ){
}


void FieldMeter::checkResources(const ResDB &rdb){
    Meter::checkResources(rdb);
    setUsedFormat(rdb.getResourceOrUseDefault(resName() + "UsedFormat",
        "percent"));
    decayUsed(rdb.isResourceTrue(resName() + "UsedDecay"));
    _used.color(rdb.getColor("usedLabelColor"));
}


void FieldMeter::setUsedFormat ( const std::string &fmt ) {
    std::string lfmt = util::tolower(fmt);
    /*  Do case-insensitive compares.  */
    if (lfmt == "percent")
        print_ = PERCENT;
    else if (lfmt == "autoscale")
        print_ = AUTOSCALE;
    else if (lfmt == "float")
        print_ = FLOAT;
    else {
        logFatal << "could not parse format of " <<  fmt << "\n"
                 << "  for meter: " << name() << "\n"
                 << "  I expected one of 'percent', 'autoscale', or 'float' \n"
                 << "  (Case-insensitive)" << std::endl;
    }
}

void FieldMeter::setUsed (float val, float total) {
    if (print_ == FLOAT)
        used_ = val;
    else if (print_ == PERCENT) {
        if (total != 0.0)
            used_ = val / total * 100.0;
        else {
            if (!printedZeroTotalMesg_) {
                printedZeroTotalMesg_ = 1;
                logProblem << name() << " meter had a zero total "
                           << "field!  Would have caused a div-by-zero "
                           << "exception." << std::endl;
            }
            used_ = 0.0;
        }
    }
    else if (print_ == AUTOSCALE)
        used_ = val;
    else {
        logFatal << "Error in " << name() << ":  I can't handle a "
                 << "UsedType enum value of " << print_ << "!" << std::endl;
    }

    updateUsed();
}


void FieldMeter::setfieldcolor( int field, const std::string &color ){
    setfieldcolor(field, parent_->g().allocColor( color ));
}


void FieldMeter::setfieldcolor( int field, unsigned long color ) {
    colors_[field] = color;
    setLegendColor(field, color);
}


void FieldMeter::draw(X11Graphics &g) {

    /*  Draw the outline for the fieldmeter.  */
    g.setFG( parent_->foreground() );
    g.drawRectangle( x() - 1, y() - 1, width() + 2, height() + 2 );

    drawLabels(g);
    _used.draw(g);
    drawfields( g, 1 );
}


void FieldMeter::drawIfNeeded(X11Graphics &g) {
    Meter::drawIfNeeded(g);
    drawfields(g, false);
    _used.drawIfNeeded(g);
}


void FieldMeter::updateUsed() {
    if (!dolegends() || !dousedlegends())
        return;

    float dispUsed = used_; // value we will display here
    if (decayUsed()) {
        _usedAvg[_usedAvgIndex%DECAYN] = dispUsed;
        _usedAvgIndex++;

        float total = 0;
        for (size_t i = 0 ; i < DECAYN ; i++)
            total += _usedAvg[i];
        dispUsed = total / (float)DECAYN;
    }

    std::ostringstream bufs;
    bufs << std::fixed;

    if (print_ == PERCENT){
        bufs << (int)dispUsed << "%";
    }
    else if (print_ == AUTOSCALE){
        char scale;
        float scaled_used;
        /*  Unfortunately, we have to do our comparisons by 1000s (otherwise
         *  a value of 1020, which is smaller than 1K, could end up
         *  being printed as 1020, which is wider than what can fit)  */
        /*  However, we do divide by 1024, so a K really is a K, and not
         *  1000.  */
        /*  In addition, we need to compare against 999.5*1000, because
         *  999.5, if not rounded up to 1.0 K, will be rounded by the
         *  %.0f to be 1000, which is too wide.  So anything at or above
         *  999.5 needs to be bumped up.  */
        if (dispUsed >= 999.5*1000*1000*1000*1000*1000*1000) {
            scale='E'; scaled_used = dispUsed/1024/1024/1024/1024/1024/1024;
        }
        else if (dispUsed >= 999.5*1000*1000*1000*1000) {
            scale='P'; scaled_used = dispUsed/1024/1024/1024/1024/1024;
        }
        else if (dispUsed >= 999.5*1000*1000*1000) {
            scale='T'; scaled_used = dispUsed/1024/1024/1024/1024;
        }
        else if (dispUsed >= 999.5*1000*1000) {
            scale='G'; scaled_used = dispUsed/1024/1024/1024;
        }
        else if (dispUsed >= 999.5*1000) {
            scale='M'; scaled_used = dispUsed/1024/1024;
        }
        else if (dispUsed >= 999.5) {
            scale='K'; scaled_used = dispUsed/1024;
        }
        else {
            scale=' '; scaled_used = dispUsed;
        }
        /*  For now, we can only print 3 characters, plus the optional
         *  suffix, without overprinting the legends.  Thus, we can
         *  print 965, or we can print 34, but we can't print 34.7 (the
         *  decimal point takes up one character).  bgrayson   */
        /*  Also check for negative values, and just print "-" for
         *  them.  */
        if (scaled_used < 0)
            bufs << "-" << scale;
        else if (scaled_used == 0.0)
            bufs << "0" << scale;
        else if (scaled_used < 9.95)  //  9.95 or above would get
                                      //  rounded to 10.0, which is too wide.
            bufs << std::setprecision(1) << scaled_used << scale;
        /*  We don't need to check against 99.5 -- it all gets %.0f.  */
        /*else if (scaled_used < 99.5)*/
        /*bufs << std::setprecision(0) << scaled_used << scale;*/
        else
            bufs << std::setprecision(0) << scaled_used << scale;
    }
    else {
        bufs << std::setprecision(1) << dispUsed;
    }

    std::string buf = bufs.str();
    _used.text(buf);
}


void FieldMeter::drawfields(X11Graphics &g, bool mandatory) {
    int twidth, x = Meter::x();

    if ( total_ == 0 )
        return;

    for ( unsigned int i = 0 ; i < numfields() ; i++ ){
        /*  Look for bogus values.  */
        logAssert(fields_[i] >= 0.0)
            << "meter " << name() <<  " had a negative "
            << "value of " << fields_[i]
            << " for field " << i << "\n"
            << "fields_: " << fields_ << std::endl;

        twidth = (int) ((width() * (float) fields_[i]) / total_);
//    twidth = (int)((fields_[i] * width_) / total_);
        if ( (i == numfields() - 1)
          && ((x + twidth) != (Meter::x() + width())) )
            twidth = width() + Meter::x() - x;

        if ( mandatory || (twidth != lastvals_[i]) || (x != lastx_[i]) ){
            g.setFG( colors_[i] );
            g.setStippleN(i%4);
            g.drawFilledRectangle( x, y(), twidth, height() );
            g.setStippleN(0);	/*  Restore all-bits stipple.  */
            lastvals_[i] = twidth;
            lastx_[i] = x;
        }

        x += twidth;
    }
}

void FieldMeter::checkevent( void ){
    drawfields(parent_->g());
}

void FieldMeter::setNumFields(size_t n){
    fields_.clear();
    fields_.resize(n, 0);
    colors_.clear();
    colors_.resize(n, 0);
    lastvals_.clear();
    lastvals_.resize(n, 0);
    lastx_.clear();
    lastx_.resize(n, 0);

    total_ = 1.0;
}
