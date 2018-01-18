//
//  Copyright (c) 1994, 1995, 2006, 2015, 2016, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "fieldmeter.h"
#include "x11graphics.h"

#include <iomanip>

// Size in sample to use for decaying used labels.
// Not scientific.  Just set to a value that caused
// the twitchy cpu meters to stop hammering the X server
// with text draws yet still give some reasonable
// update rate.
static const size_t DECAYN = 8;



FieldMeter::FieldMeter(size_t numfields, const std::string &title,
  const std::string &legend)
    : Meter(title, legend),
      _fields(numfields, 0.0), _total(1.0),
      _lastvals(numfields, 0.0),
      _lastx(numfields, 0), _used(0), _colors(numfields, 0),
      _usedFmt(PERCENT), _printedZeroTotalMsg(false),
      _usedAvg(DECAYN, 0.0), _usedAvgIndex(0),
      _decayUsed(false), _usedLabel(0, 0, Label::BLSE) {
}


void FieldMeter::resize( int x, int y, int width, int height ) {
    Meter::resize(x, y, width, height);
    _usedLabel.move(Meter::x() - 2, Meter::y() + Meter::height() + 2);
}


FieldMeter::~FieldMeter( void ){
}


void FieldMeter::checkResources(const ResDB &rdb){
    Meter::checkResources(rdb);
    setUsedFormat(rdb.getResourceOrUseDefault(resName() + "UsedFormat",
        "percent"));
    decayUsed(rdb.isResourceTrue(resName() + "UsedDecay"));
    _usedLabel.color(rdb.getColor("usedLabelColor"));
}


void FieldMeter::setUsedFormat ( const std::string &fmt ) {
    std::string lfmt = util::tolower(fmt);
    /*  Do case-insensitive compares.  */
    if (lfmt == "percent")
        _usedFmt = PERCENT;
    else if (lfmt == "autoscale")
        _usedFmt = AUTOSCALE;
    else if (lfmt == "float")
        _usedFmt = FLOAT;
    else {
        logFatal << "could not parse format of " <<  fmt << "\n"
                 << "  for meter: " << name() << "\n"
                 << "  I expected one of 'percent', 'autoscale', or 'float' \n"
                 << "  (Case-insensitive)" << std::endl;
    }
}

void FieldMeter::setUsed (float val, float total) {
    if (_usedFmt == FLOAT)
        _used = val;
    else if (_usedFmt == PERCENT) {
        if (total != 0.0)
            _used = val / total * 100.0;
        else {
            if (!_printedZeroTotalMsg) {
                _printedZeroTotalMsg = true;
                logProblem << name() << " meter had a zero total "
                           << "field!  Would have caused a div-by-zero "
                           << "exception." << std::endl;
            }
            _used = 0.0;
        }
    }
    else if (_usedFmt == AUTOSCALE)
        _used = val;
    else {
        logFatal << "Error in " << name() << ":  I can't handle a "
                 << "UsedType enum value of " << _usedFmt << "!" << std::endl;
    }

    updateUsed();
}


void FieldMeter::setfieldcolor(size_t field, unsigned long color) {
    _colors[field] = color;
    setLegendColor(field, color);
}


void FieldMeter::draw(X11Graphics &g) {

    /*  Draw the outline for the fieldmeter.  */
    g.setFG( fgColor() );
    g.drawRectangle( fldx() - 1, y() - 1, fldwidth() + 2, height() + 2 );

    drawLabels(g);
    _usedLabel.draw(g);
    drawfields( g, true );
}


void FieldMeter::drawIfNeeded(X11Graphics &g) {
    Meter::drawIfNeeded(g);
    drawfields(g, false);
    _usedLabel.drawIfNeeded(g);
}


void FieldMeter::updateUsed() {
    if (!dolegends() || !dousedlegends())
        return;

    float dispUsed = _used; // value we will display here
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

    if (_usedFmt == PERCENT){
        bufs << (int)dispUsed << "%";
    }
    else if (_usedFmt == AUTOSCALE){
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

    _usedLabel.text(bufs.str());
}


void FieldMeter::drawfields(X11Graphics &g, bool mandatory) {
    const int fx = fldx();
    const int fwidth = fldwidth();
    int twidth, x = fx;

    if ( _total == 0 )
        return;

    for ( unsigned int i = 0 ; i < numfields() ; i++ ){
        /*  Look for bogus values.  */
        logAssert(_fields[i] >= 0.0)
            << "meter " << name() <<  " had a negative "
            << "value of " << _fields[i]
            << " for field " << i << "\n"
            << "_fields: " << _fields << std::endl;

        twidth = (int) ((fwidth * (float) _fields[i]) / _total);
        if ( (i == numfields() - 1)
          && ((x + twidth) != (fx + fwidth)) )
            twidth = fwidth + fx - x;

        if ( mandatory || (twidth != _lastvals[i]) || (x != _lastx[i]) ){
            g.setFG( _colors[i] );
            g.setStippleN(i%4);
            g.drawFilledRectangle( x, y(), twidth, height() );
            g.setStippleN(0);	/*  Restore all-bits stipple.  */
            _lastvals[i] = twidth;
            _lastx[i] = x;
        }

        x += twidth;
    }
}


void FieldMeter::setNumFields(size_t n){
    _fields.clear();
    _fields.resize(n, 0);
    _colors.clear();
    _colors.resize(n, 0);
    _lastvals.clear();
    _lastvals.resize(n, 0);
    _lastx.clear();
    _lastx.resize(n, 0);

    _total = 1.0;
}
