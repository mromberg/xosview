//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "fieldmeter.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

// Size in sample to use for decaying used labels.
// Not scientific.  Just set to a value that caused
// the twitchy cpu meters to stop hammering the X server
// with text draws yet still give some reasonable
// update rate.
static const size_t DECAYN = 25;

FieldMeter::FieldMeter(XOSView *parent, size_t numfields,
  const std::string &title,
  const std::string &legend, bool docaptions, bool dolegends,
  bool dousedlegends)
    : Meter(parent, title, legend, docaptions, dolegends, dousedlegends),
      fields_(numfields, 0.0), total_(1.0),
      used_(0), lastvals_(numfields, 0.0),
      lastx_(numfields, 0), colors_(numfields, 0),
      usedcolor_(0), print_(PERCENT), printedZeroTotalMesg_(0),
      numWarnings_(0), _usedAvg(DECAYN, 0.0), _usedAvgIndex(0),
      _decayUsed(false) {
}

void FieldMeter::disableMeter(void) {
    setNumFields(1);
    setfieldcolor (0, "gray");
    Meter::legend ("Disabled");
    // And specify the total of 1.0, so the meter is grayed out.
    total_ = 1.0;
    fields_[0] = 1.0;
}


FieldMeter::~FieldMeter( void ){
}

void FieldMeter::checkResources( void ){
    Meter::checkResources();
    usedcolor_ = parent_->g().allocColor(parent_->getResource(
          "usedLabelColor"));
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
                 << "  I expected one of 'percent', 'bytes', or 'float' \n"
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
}

void FieldMeter::reset( void ){
    for ( unsigned int i = 0 ; i < numfields() ; i++ )
        lastvals_[i] = lastx_[i] = -1;
}

void FieldMeter::setfieldcolor( int field, const std::string &color ){
    colors_[field] = parent_->g().allocColor( color );
}

void FieldMeter::setfieldcolor( int field, unsigned long color ) {
    colors_[field] = color;
}

void FieldMeter::draw(X11Graphics &g) {

    /*  Draw the outline for the fieldmeter.  */
    g.setFG( parent_->foreground() );
    g.drawRectangle( x_ - 1, y_ - 1, width_ + 2, height_ + 2 );

    drawLabels(g);
    drawfields( g, 1 );
}

void FieldMeter::drawLegend(X11Graphics &g) {
    if (!dolegends() || !docaptions())
        return;
    // (x_, y_) = coord of upper left of fields
    // the outline overlaps (x_, y_)
    // So make the text draw on y_ - 1 - desent of the text (y=0 is top of)
    //
    // x + textWidth() is the location to start the next glyph
    //
    // And something funky is going on (I think) with the width
    // height of rectangles/clear areas.  off by one.
    size_t pos = 0;
    int x = x_;
    int y = y_ - 1 - g.textDescent() - 1; // the bonus -1 I can't explain

    g.clear(x, y - g.textAscent(), g.textWidth(legend()),
      g.textHeight()+1); // again the bonus + 1 is a mystery

    for (unsigned int i = 0 ; i < numfields() ; i++) {
        size_t fpos = legend_.find("/", pos); // string::npos if not found
        std::string li = legend_.substr(pos, fpos - pos);
        pos = fpos + 1;

        g.setStippleN(i%4);
        g.setFG( colors_[i] );
        g.drawString( x, y, li);
        x += g.textWidth( li );

        g.setFG( parent_->foreground() );
        if ( i != numfields() - 1 )
            g.drawString( x, y, "/" );
        x += g.textWidth("/");
    }
    g.setStippleN(0);	/*  Restore default all-bits stipple.  */
}

void FieldMeter::drawused(X11Graphics &g, bool manditory) {
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

    g.setStippleN(0);	/*  Use all-bits stipple.  */

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
    if (!manditory && (buf == _lastUsedStr))
        return;

    unsigned int twidth = g.textWidth(buf);
    unsigned int cwidth = std::max(g.textWidth(_lastUsedStr), twidth);
    unsigned int sheight = g.textAscent();
    int sx = x_ - (cwidth + 2);
    int tx = x_ - (twidth + 2);
    int sy = y_ + height_ + 1;

    g.clear(sx, sy-g.textAscent(), cwidth+1, sheight+1);
    g.setFG( usedcolor_ );

    // drawing text is expensive on the X server
    // If this message is too annoying someting is drawing too much
    // uncomment to check
    //logDebug << "draw used: " << name() << std::endl;
    g.drawString( tx, sy, buf);

    _lastUsedStr = buf;
}

void FieldMeter::drawfields(X11Graphics &g, bool manditory) {
    int twidth, x = x_;

    if ( total_ == 0 )
        return;

    for ( unsigned int i = 0 ; i < numfields() ; i++ ){
        /*  Look for bogus values.  */
        if (fields_[i] < 0.0) {
            /*  Only print a warning 5 times per meter, followed by a
             *  message about no more warnings.  */
            numWarnings_ ++;
            if (numWarnings_ < 5)
                logProblem << "meter " << name() <<  " had a negative "
                           << "value of %f for field " << fields_[i]
                           << std::endl;
            if (numWarnings_ == 5)
                logProblem << "Future warnings from the " << name() << " meter "
                           << "will not be displayed." << std::endl;
        }

        twidth = (int) ((width_ * (float) fields_[i]) / total_);
//    twidth = (int)((fields_[i] * width_) / total_);
        if ( (i == numfields() - 1) && ((x + twidth) != (x_ + width_)) )
            twidth = width_ + x_ - x;

        if ( manditory || (twidth != lastvals_[i]) || (x != lastx_[i]) ){
            g.setFG( colors_[i] );
            g.setStippleN(i%4);
            g.drawFilledRectangle( x, y_, twidth, height_ );
            g.setStippleN(0);	/*  Restore all-bits stipple.  */
            lastvals_[i] = twidth;
            lastx_[i] = x;

            drawused( g, manditory );
        }
        x += twidth;
    }

    //parent_->flush();
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

bool FieldMeter::checkX(int x, int width) const {
    if ((x < x_) || (x + width < x_)
      || (x > x_ + width_) || (x + width > x_ + width_)){
        logProblem << "FieldMeter::checkX() : bad horiz values for meter : "
                   << name() << std::endl
                   << "value " << x << ", width " << width << ", total_ = "
                   << total_ << std::endl;

        for (unsigned int i = 0 ; i < numfields() ; i++)
            logProblem << "fields_[" << i << "] = " << fields_[i] << std::endl;

        return false;
    }

    return true;
}
