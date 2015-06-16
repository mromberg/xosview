//
//  Copyright (c) 1999, 2006, 2015
//  by Thomas Waldmann ( ThomasWaldmann@gmx.de )
//  based on work of Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
//
#ifdef HAVE_FSTREAM
#include <fstream>
#else
#include <fstream.h>
#endif
#include <sstream>
#include <iomanip>
#include <stdlib.h>
#include "general.h"
#include "bitfieldmeter.h"
#include "xosview.h"


BitFieldMeter::BitFieldMeter( XOSView *parent, int numBits, int numfields,
  const std::string &title,
  const std::string &bitslegend, const std::string &FieldLegend,
  int docaptions, int dolegends, int dousedlegends )
    : Meter(parent, title, bitslegend, docaptions, dolegends, dousedlegends){
    /*  We need to set print_ to something valid -- the meters
     *  apparently get drawn before the meters have a chance to call
     *  CheckResources() themselves.
     */
    numWarnings_ = printedZeroTotalMesg_ = 0;
    print_ = PERCENT;
    used_ = 0;
    lastused_ = -1;
    setNumBits(numBits);
    setfieldlegend(FieldLegend);
    setNumFields(numfields);
}

void BitFieldMeter::disableMeter ( ) {
    setNumFields(1);
    setfieldcolor (0, "grey");
    setfieldlegend ("Disabled");
    total_ = fields_[0] = 1.0;
    setNumBits(1);
    offColor_ = onColor_ = parent_->allocColor("grey");
}


BitFieldMeter::~BitFieldMeter( void ){
}

void BitFieldMeter::checkResources( void ){
    Meter::checkResources();
    usedcolor_ = parent_->allocColor( parent_->getResource( "usedLabelColor") );
}


void BitFieldMeter::setNumBits(int n){
    bits_.resize(n);
    lastbits_.resize(n);

    for ( unsigned int i = 0 ; i < numbits() ; i++ )
        bits_[i] = lastbits_[i] = 0;
}

void BitFieldMeter::SetUsedFormat ( const std::string &fmt ) {
    /*  Do case-insensitive compares.  */
    if (!strncasecmp (fmt.c_str(), "percent", 8))
        print_ = PERCENT;
    else if (!strncasecmp (fmt.c_str(), "autoscale", 10))
        print_ = AUTOSCALE;
    else if (!strncasecmp (fmt.c_str(), "float", 6))
        print_ = FLOAT;
    else {
        std::cerr << "Error:  could not parse format of " <<  fmt << "\n"
                  << "  I expected one of 'percent', 'bytes', or 'float'\n"
                  << "  (Case-insensitive)" << std::endl;
        exit(1);
    }
}

void BitFieldMeter::setUsed (float val, float total) {
    if (print_ == FLOAT)
        used_ = val;
    else if (print_ == PERCENT) {
        if (total != 0.0)
            used_ = val / total * 100.0;
        else {
            if (!printedZeroTotalMesg_) {
                printedZeroTotalMesg_ = 1;
                std::cerr << "Warning:  " << name()
                          << " meter had a zero total "
                          << "field!  Would have caused a div-by-zero "
                          << "exception." << std::endl;
            }
            used_ = 0.0;
        }
    }
    else if (print_ == AUTOSCALE)
        used_ = val;
    else {
        std::cerr << "Error in " << name() << ":  I can't handle a "
                  << "UsedType enum value of " << print_ << "!" << std::endl;
        exit(1);
    }
}

void BitFieldMeter::reset( void ){
    for ( unsigned int i = 0 ; i < numfields() ; i++ )
        lastvals_[i] = lastx_[i] = -1;
}

void BitFieldMeter::setfieldcolor( int field, const std::string &color ){
    colors_[field] = parent_->allocColor( color );
}

void BitFieldMeter::setfieldcolor( int field, unsigned long color ) {
    colors_[field] = color;
}


void BitFieldMeter::draw( void ){
    /*  Draw the outline for the fieldmeter.  */
    parent_->setForeground( parent_->foreground() );
    parent_->lineWidth( 1 );
    parent_->drawFilledRectangle( x_ - 1, y_ - 1, width_/2 + 2, height_ + 2 );

    parent_->drawRectangle( x_ + width_/2 +3, y_ - 1, width_/2 - 2,
      height_ + 2 );
    if ( dolegends_ ){
        parent_->setForeground( textcolor_ );

        int offset;
        if ( dousedlegends_ )
            offset = parent_->textWidth( "XXXXXXXXX" );
        else
            offset = parent_->textWidth( "XXXXX" );

        parent_->drawString( x_ - offset + 1, y_ + height_, title_ );

        if(docaptions_){
            parent_->setForeground( onColor_ );
            parent_->drawString( x_, y_ - 5, legend_ );
            drawfieldlegend();
        }
    }
    drawBits( 1 );
    drawfields( 1 );
}

void BitFieldMeter::drawfieldlegend( void ){
    size_t pos = 0;
    int x = x_ + width_/2 + 4;

    for (unsigned int i = 0 ; i < numfields() ; i++) {
        size_t fpos = fieldLegend_.find("/", pos);  // string::npos if not found
        std::string li = fieldLegend_.substr(pos, fpos - pos);
        pos = fpos + 1;

        parent_->setStippleN(i%4);
        parent_->setForeground( colors_[i] );
        parent_->drawString( x, y_ - 5, li );
        x += parent_->textWidth( li );
        parent_->setForeground( parent_->foreground() );
        if ( i != numfields() - 1 )
            parent_->drawString( x, y_ - 5, "/" );
        x += parent_->textWidth( "/" );
    }

    parent_->setStippleN(0);	/*  Restore default all-bits stipple.  */
}

void BitFieldMeter::drawused( int manditory ){
    if ( !manditory )
        if ( (lastused_ == used_) )
            return;

    parent_->setStippleN(0);	/*  Use all-bits stipple.  */
    static const int onechar = parent_->textWidth( "X" );
    static int xoffset = parent_->textWidth( "XXXXX" );

    std::ostringstream bufs;
    bufs << std::fixed;

    if (print_ == PERCENT){
        bufs << (int)used_ << "%";
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
        if (used_ >= 999.5*1000*1000*1000*1000*1000*1000) {
            scale='E'; scaled_used = used_/1024/1024/1024/1024/1024/1024;
        }
        else if (used_ >= 999.5*1000*1000*1000*1000) {
            scale='P'; scaled_used = used_/1024/1024/1024/1024/1024;
        }
        else if (used_ >= 999.5*1000*1000*1000) {
            scale='T'; scaled_used = used_/1024/1024/1024/1024;
        }
        else if (used_ >= 999.5*1000*1000) {
            scale='G'; scaled_used = used_/1024/1024/1024;
        }
        else if (used_ >= 999.5*1000) {
            scale='M'; scaled_used = used_/1024/1024;
        }
        else if (used_ >= 999.5) {
            scale='K'; scaled_used = used_/1024;
        }
    else {
        scale=' '; scaled_used = used_;
    }
        /*  For now, we can only print 3 characters, plus the optional
         *  suffix, without overprinting the legends.  Thus, we can
         *  print 965, or we can print 34, but we can't print 34.7 (the
         *  decimal point takes up one character).  bgrayson   */
        if (scaled_used == 0.0)
            bufs << "0";
        else if (scaled_used < 9.95)  //  9.95 or above would get
                                      //  rounded to 10.0, which is too wide.
            bufs << std::setprecision(1) << scaled_used << scale;
        /*  We don't need to check against 99.5 -- it all gets %.0f.  */
        /*else if (scaled_used < 99.5)*/
        /* bufs << std::setprecision(0) << scaled_used << scale; */
        else
            bufs << std::setprecision(0) << scaled_used << scale;
    }
    else {
        bufs << std::setprecision(1) << used_;
    }

    parent_->clear( x_ - xoffset, y_ + height_ - parent_->textHeight(),
      xoffset - onechar / 2, parent_->textHeight() + 1 );
    parent_->setForeground( usedcolor_ );
    std::string buf = bufs.str();
    parent_->drawString( x_ - (buf.size() + 1 ) * onechar + 2,
      y_ + height_, buf );

    lastused_ = used_;
}

void BitFieldMeter::drawBits( int manditory ){
    static int pass = 1;

    int x1 = x_, w;

    w = (width_/2 - (numbits()+1)) / numbits();

    for ( unsigned int i = 0 ; i < numbits() ; i++ ){
        if ( (bits_[i] != lastbits_[i]) || manditory ){
            if ( bits_[i] && pass )
                parent_->setForeground( onColor_ );
            else
                parent_->setForeground( offColor_ );
            parent_->drawFilledRectangle( x1, y_, w, height_);
        }

        lastbits_[i] = bits_[i];

        x1 += (w + 2);
    }
}

void BitFieldMeter::drawfields( int manditory ){
    int twidth, x = x_ + width_/2 + 4;

    if ( total_ == 0 )
        return;

    for ( unsigned int i = 0 ; i < numfields() ; i++ ){
        /*  Look for bogus values.  */
        if (fields_[i] < 0.0) {
            /*  Only print a warning 5 times per meter, followed by a
             *  message about no more warnings.  */
            numWarnings_ ++;
            if (numWarnings_ < 5)
                std::cerr << "Warning:  meter " << name() << " had a negative "
                          << "value of " << fields_[i] << " for field " << i
                          << std::endl;
            if (numWarnings_ == 5)
                std::cerr << "Future warnings from the " << name() << " meter "
                          << "will not be displayed." << std::endl;
        }

        twidth = (int) (((width_/2 - 3) * (float) fields_[i]) / total_);
        if ( (i == numfields() - 1) && ((x + twidth) != (x_ + width_)) )
            twidth = width_ + x_ - x;

        if ( manditory || (twidth != lastvals_[i]) || (x != lastx_[i]) ){
            parent_->setForeground( colors_[i] );
            parent_->setStippleN(i%4);
            parent_->drawFilledRectangle( x, y_, twidth, height_ );
            parent_->setStippleN(0);	/*  Restore all-bits stipple.  */
            lastvals_[i] = twidth;
            lastx_[i] = x;

            if ( dousedlegends_ )
                drawused( manditory );
        }
        x += twidth;
    }

    //parent_->flush();
}

void BitFieldMeter::checkevent( void ){
    drawBits();
    drawfields();
}

void BitFieldMeter::setBits(int startbit, unsigned char values){
    unsigned char mask = 1;
    for (int i = startbit ; i < startbit + 8 ; i++){
        bits_[i] = values & mask;
        mask = mask << 1;
    }
}

void BitFieldMeter::setNumFields(int n){
    fields_.resize(n);
    colors_.resize(n);
    lastvals_.resize(n);
    lastx_.resize(n);

    total_ = 0;
    for ( unsigned int i = 0 ; i < numfields() ; i++ ){
        fields_[i] = 0.0;
        lastvals_[i] = lastx_[i] = 0;
    }
}

bool BitFieldMeter::checkX(int x, int width) const {
    if ((x < x_) || (x + width < x_)
      || (x > x_ + width_) || (x + width > x_ + width_)){
        std::cerr << "BitFieldMeter::checkX() : bad horiz values for meter : "
                  << name() << std::endl;

        std::cerr <<"value "<<x<<", width "<<width<<", total_ = "
                  <<total_<<std::endl;

        for (unsigned int i = 0 ; i < numfields() ; i++)
            std::cerr <<"fields_[" <<i <<"] = " <<fields_[i] <<",";
        std::cerr <<std::endl;

        return false;
    }

    return true;
}
