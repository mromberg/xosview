//
//  The original FieldMeter class is Copyright (c) 1994, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  Modifications from FieldMeter class done in Oct. 1995
//    by Brian Grayson ( bgrayson@netbsd.org )
//
//  Modifications from FieldMeterDecay class done in Oct. 1998
//    by Scott McNab ( jedi@tartarus.uwa.edu.au )
//


// In order to use the FieldMeterGraph class in place of a FieldMeter class in
// a meter file (say, cpumeter.cc), make the following changes:
//   1.  Change cpumeter.h to include fieldmetergraph.h instead of
//       fieldmeter.h
//   2.  Change CPUMeter to inherit from FieldMeterGraph, rather than
//       FieldMeter.
//   3.  Change the constructor call to use FieldMeterGraph(), rather than
//       FieldMeter().
//   4.  Make the meter call FieldMeterGraph::checkResources(),
//       to pick up graphNumCols resource.
//   5.  Make the checkResources () function in the meter set the
//	 useGraph_ variable according to the, e.g., xosview*cpuGraph resource.

#ifdef HAVE_FSTREAM
#include <fstream>
#else
#include <fstream.h>
#endif
#include <math.h>		//  For fabs()
#include "general.h"
#include "fieldmeter.h"
#include "fieldmetergraph.h"
#include "xosview.h"


FieldMeterGraph::FieldMeterGraph( XOSView *parent,
  int numfields, const std::string &title,
  const std::string &legend, int docaptions, int dolegends,
  int dousedlegends )
    : FieldMeterDecay (parent, numfields, title, legend, docaptions,
      dolegends, dousedlegends) {

    useGraph_ = 0;
    firstTimeDrawn_ = 1;

    // set number of columns to a reasonable default in case we can't
    // find the resource
    setNumCols( 100 );
}

FieldMeterGraph::~FieldMeterGraph( void ){
}

void FieldMeterGraph::drawfields( int manditory ){
    drawBars(parent_->g(), manditory);
}

void FieldMeterGraph::drawBars(X11Graphics &g, int manditory) {
    int i;

    if( !useGraph_ ) {
        // Call FieldMeterDecay code if this meter should not be
        // drawn as a graph
        FieldMeterDecay::drawfields( manditory );
        return;
    }

    if( total_ <= 0.0 )
        return;

    // allocate memory for height field graph storage
    // note: this is done here as it is not certain that both
    // numfields_ and graphNumCols_ are defined in the constructor
    if( heightfield_.size() == 0 ) {
        if( numfields() > 0 && graphNumCols_ > 0 ) {
            heightfield_.resize(numfields()*graphNumCols_);

            for( i = 0; i < graphNumCols_; i++ ) {
                for( unsigned int j = 0; j < numfields(); j++ ) {
                    if( j < numfields()-1 )
                        heightfield_[i*numfields()+j] = 0.0;
                    else
                        heightfield_[i*numfields()+j] = 1.0;
                }
            }
        }
    }

    // check current position here and slide graph if necessary
    if( graphpos_ >= graphNumCols_ ) {
        for( i = 0; i < graphNumCols_-1; i++ ) {
            for( unsigned int j = 0; j < numfields(); j++ ) {
                heightfield_[i*numfields()+j] =
                    heightfield_[(i+1)*numfields()+j];
            }
        }
        graphpos_ = graphNumCols_ - 1;
    }

    // get current values to be plotted
    for( unsigned int i = 0; i < numfields(); i++ ) {
        float a = fields_[i] / total_;
        if( a <= 0.0 )
            a = 0.0;
        if( a >= 1.0 )
            a = 1.0;
        heightfield_[graphpos_*numfields()+i] = a;
    }

    /*  For the first time, we need to draw everything, so
     *  skip the optimized copyArea case.  Also, if we are
     *  not fully visible, then the copy-area won't work
     *  properly.  */
    if( !firstTimeDrawn_ && parent_->hasBeenExposedAtLeastOnce()
      && !parent_->isExposed() && parent_->isFullyVisible() ) {
        // scroll area
        int col_width = width_/graphNumCols_;
        if( col_width < 1 ) {
            col_width = 1;
        }

        int sx = x_ + col_width;
        int swidth = width_ - col_width;
        int sheight = height_ + 1;
        if( sx > x_ && swidth > 0 && sheight > 0 )
            g.copyArea( sx, y_, swidth, sheight, x_, y_ );
        drawBar(g, graphNumCols_ - 1);
    } else {
        if (firstTimeDrawn_ &&
          parent_->isAtLeastPartiallyVisible() &&
          parent_->hasBeenExposedAtLeastOnce()) {
            logDebug << "True exposure! " << firstTimeDrawn_ << std::endl;
            firstTimeDrawn_ = 0;
        }
        else
            logDebug << "Full draw:  isAtLeastPart "
                     << parent_->isAtLeastPartiallyVisible()
                     << ", hasBeenExposed "
                     << parent_->hasBeenExposedAtLeastOnce() << std::endl;

        // need to draw entire graph on expose event
        for( i = 0; i < graphNumCols_; i++ ) {
            drawBar(g, i);
        }
    }

    graphpos_++;
    g.setStippleN(0);	//  Restore all-bits stipple.
    if ( dousedlegends_ ) {
    	drawused( manditory );
    }
}


void FieldMeterGraph::drawBar(X11Graphics &g, int i) {
    int y = y_ + height_;
    int x = x_ + i*width_/graphNumCols_;
    int barwidth = (x_ + (i+1)*width_/graphNumCols_)-x;

    if( barwidth>0 ) {
        int barheight;
        for( unsigned int j = 0 ; j < numfields(); j++ ) {
            /*  Round up, by adding 0.5 before
             *  converting to an int.  */
            barheight = (int)((heightfield_[i*numfields()+j]*height_)+0.5);

            g.setForeground( colors_[j] );
            g.setStippleN(j%4);

            if( barheight > (y-y_) )
                barheight = (y-y_);

            // hack to ensure last field always reaches top of graph area
            if( j == numfields()-1 )
                barheight = (y-y_);

            y -= barheight;
            if( barheight>0 )
                g.drawFilledRectangle( x, y, barwidth, barheight );
        }
    }
}

void FieldMeterGraph::checkResources( void ) {
    FieldMeterDecay::checkResources();

    // exit(1) if does not exist
    std::string ptr = parent_->getResource( "graphNumCols" );

    int i;
    if (util::fstr(ptr, i)) {
        if( i>0 ) {
            setNumCols( i );
        }
    }
}

void FieldMeterGraph::setNumCols( int n ) {
    graphNumCols_ = n;
    graphpos_ = graphNumCols_-1;

    heightfield_.resize(0);
}
