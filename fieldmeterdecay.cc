//
//  The original FieldMeter class is Copyright (c) 1994, 2006, 2015, 2016, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  Modifications from FieldMeter class done in Oct. 1995
//    by Brian Grayson ( bgrayson@netbsd.org )
//
//  This file was written by Brian Grayson for the NetBSD and xosview
//    projects.
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//


// In order to use the FieldMeterDecay class in place of a FieldMeter class in
// a meter file (say, cpumeter.cc), make the following changes:
//   1.  Change cpumeter.h to include fieldmeterdecay.h instead of
//       fieldmeter.h
//   2.  Change CPUMeter to inherit from FieldMeterDecay, rather than
//       FieldMeter.
//   3.  Change the constructor call to use FieldMeterDecay(), rather than
//       FieldMeter().
//   4.  Make the checkResources () function in the meter set the
//	 dodecay_ variable according to the, e.g., xosview*cpuDecay resource.
#include "fieldmeterdecay.h"
#include "x11graphics.h"



FieldMeterDecay::FieldMeterDecay(size_t numfields, const std::string &title,
  const std::string &legend)
    : FieldMeter (numfields, title, legend),
      dodecay_(true), firsttime_(true), decay_(numfields, 0.0),
      lastDecayval_(numfields, 0.0) {

    decay_[numfields-1] = 1.0;  //  Initialize to all free...
}


FieldMeterDecay::~FieldMeterDecay( void ){
}


void FieldMeterDecay::checkResources(const ResDB &rdb) {
    FieldMeter::checkResources(rdb);
    dodecay_ = rdb.isResourceTrue( resName() + "Decay" );
}


void FieldMeterDecay::drawfields(X11Graphics &g, bool mandatory) {

    if (!dodecay_) {
        //  If this meter shouldn't be done as a decaying splitmeter,
        //  call the ordinary fieldmeter code.
        FieldMeter::drawfields(g, mandatory);
        return;
    }

    if ( _total == 0.0 )
        return;

    const int fx = fldx();
    const int fwidth = fldwidth();

    //  This code is supposed to make the average display look just like
    //  the ordinary display for the first drawfields, but it doesn't seem
    //  to work too well.  But it's better than setting all decay_ fields
    //  to 0.0 initially!

    if (firsttime_) {
        firsttime_ = false;
        for (unsigned int i = 0; i < numfields(); i++)
            decay_[i] = _fields[i] / _total;
    }

    //  Update the decay fields.  This is not quite accurate, since if
    //  the screen is refreshed, we will update the decay fields more
    //  often than we need to.  However, this makes the decay stuff
    //  TOTALLY independent of the ????Meter methods.

    //  The constant below can be modified for quicker or slower
    //  exponential rates for the average.  No fancy math is done to
    //  set it to correspond to a five-second decay or anything -- I
    //  just played with it until I thought it looked good!  :)  BCG
    const float ALPHA = 0.97;

    /*  This is majorly ugly code.  It needs a rewrite.  BCG  */
    /*  I think one good way to do it may be to normalize all of the
     *  _fields in a temporary array into the range 0.0 .. 1.0,
     *  calculate the shifted starting positions and ending positions
     *  for coloring, multiply by the pixel width of the meter, and
     *  then turn to ints.  I think this will solve a whole bunch of
     *  our problems with rounding that before we tackled at a whole
     *  lot of places.  BCG */
    const int halfheight = height() / 2;
    int decayx = fx;
    int x = fx;
    for ( size_t i = 0 ; i < numfields() ; i++ ){

        decay_[i] = ALPHA * decay_[i] + (1.0 - ALPHA) * (_fields[i] / _total);

        //  We want to round the widths, rather than truncate.
        int twidth = (int) (0.5 + (fwidth * (float) _fields[i]) / _total);
        int decaytwidth = (int) (0.5 + fwidth * decay_[i]);
        logAssert(decaytwidth >= 0.0)
            << "FieldMeterDecay " << name()
            << ":  decaytwidth of " << std::endl
            << decaytwidth << ", width of " << fwidth
            << ", decay_[" << i << std::endl
            << "] of " << decay_[i] << std::endl;

        //  However, due to rounding, we may have gone one
        //  pixel too far by the time we get to the later fields...
        if (x + twidth > fx + fwidth)
            twidth = fwidth + fx - x;
        if (decayx + decaytwidth > fx + fwidth)
            decaytwidth = fwidth + fx - decayx;

        //  Also, due to rounding error, the last field may not go far
        //  enough...
        if ( (i == numfields() - 1)
          && ((x + twidth) != (fx + fwidth)) )
            twidth = fwidth + fx - x;
        if ( (i == numfields() - 1)
          && ((decayx + decaytwidth) != (fx + fwidth)))
            decaytwidth = fwidth + fx - decayx;

        //  drawFilledRectangle() adds one to its width and height.
        //    Let's correct for that here.
        bool fgSet = false;
        if ( mandatory || (twidth != _lastvals[i]) || (x != _lastx[i]) ){
            checkX(x, twidth);
            g.setFG( fieldcolor(i) );
            g.setStippleN(i%4);
            fgSet = true;
            g.drawFilledRectangle( x, y(), twidth, halfheight );
            _lastvals[i] = twidth;
            _lastx[i] = x;
        }

        if ( mandatory || (decay_[i] != lastDecayval_[i]) ){
            checkX(decayx, decaytwidth);
            if (!fgSet) {
                g.setFG( fieldcolor(i) );
                g.setStippleN(i%4);
            }
            g.drawFilledRectangle( decayx, y() + halfheight + 1,
              decaytwidth, height() - halfheight - 1);
            lastDecayval_[i] = decay_[i];
        }

        x += twidth;
        decayx += decaytwidth;
    }

    g.setStippleN(0);	/*  Restore all-bits stipple.  */
}
