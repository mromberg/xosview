//  
//  The original FieldMeter class is Copyright (c) 1994 by Mike Romberg 
//    ( romberg@fsl.noaa.gov )
//
//  Modifications from FieldMeter class done in Oct. 1995
//    by Brian Grayson ( bgrayson@ece.utexas.edu )
//
//  Modifications from FieldMeterDecay class done in Oct. 1998
//    by Scott McNab ( jedi@tartarus.uwa.edu.au )
//
// $Id: fieldmetergraph.cc,v 1.1 1998/10/15 21:28:16 mromberg Exp $
//

// In order to use the FieldMeterGraph class in place of a FieldMeter class in
// a meter file (say, cpumeter.cc), make the following changes:
//   1.  Change cpumeter.h to include fieldmetergraph.h instead of
//       fieldmeter.h
//   2.  Change CPUMeter to inherit from FieldMeterGraph, rather than
//       FieldMeter.
//   3.  Change the constructor call to use FieldMeterGraph(), rather than
//       FieldMeter().
//   4.  Make the checkResources () function in the meter set the 
//	 useGraph_ variable according to the, e.g., xosview*cpuGraph resource.

#include <fstream.h>
#include <math.h>		//  For fabs()
#include "general.h"
#include "fieldmeter.h"
#include "fieldmetergraph.h"
#include "xosview.h"

CVSID("$Id: fieldmetergraph.cc,v 1.1 1998/10/15 21:28:16 mromberg Exp $");
CVSID_DOT_H(FIELDMETERGRAPH_H_CVSID);

FieldMeterGraph::FieldMeterGraph( XOSView *parent,
                int numfields, const char *title,
                const char *legend, int dolegends, int dousedlegends )
: FieldMeterDecay (parent, numfields, title, legend, dolegends, dousedlegends)
{

	useGraph_ = 0;
	heightfield_ = NULL;
	
	// set number of columns to a reasonable default in case we can't
	// find the resource
	setNumCols( 100 );

}

FieldMeterGraph::~FieldMeterGraph( void )
{
	delete [] heightfield_;
}

void FieldMeterGraph::drawfields( int manditory )
{
	int i,j;

	if( !useGraph_ )
	{
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
	if( heightfield_ == NULL )
	{
		if( numfields_ > 0 && graphNumCols_ > 0 )
		{
			heightfield_ = new float [numfields_*graphNumCols_];
	
			for( i = 0; i < graphNumCols_; i++ )
			{
				for( j = 0; j < numfields_; j++ )
				{
					if( j < numfields_-1 )
						heightfield_[i*numfields_+j] = 0.0;
					else
						heightfield_[i*numfields_+j] = 1.0;
				}
			}
		}
	}

	// check current position here and slide graph if necessary
	if( graphpos_ >= graphNumCols_ )
	{
		for( i = 0; i < graphNumCols_-1; i++ )
		{
			for( j = 0; j < numfields_; j++ )
			{
				heightfield_[i*numfields_+j] = heightfield_[(i+1)*numfields_+j];
			}
		}
		graphpos_ = graphNumCols_ - 1;
	}

	// get current values to be plotted
	for( i = 0; i < numfields_; i++ )
	{
		float a = fields_[i] / total_;
		if( a <= 0.0 )
			a = 0.0;
		if( a >= 1.0 )
			a = 1.0;
		heightfield_[graphpos_*numfields_+i] = a;
	}

	for( i = 0; i < graphNumCols_; i++ )
	{
		int y = y_ + height_;
		int x = x_ + i*width_/graphNumCols_;
		int barwidth = (x_ + (i+1)*width_/graphNumCols_)-x;

		if( barwidth>0 )
		{
			int barheight;
			for( j = 0 ; j < numfields_; j++ )
			{
				barheight = (int)(heightfield_[i*numfields_+j]*height_);

    			parent_->setForeground( colors_[j] );
    			parent_->setStippleN(j%4);

    			if( barheight > (y-y_) )
    				barheight = (y-y_);

				// hack to ensure last field always reaches top of graph area
				if( j == numfields_-1 )
					barheight = (y-y_);
	
    			y -= barheight;
    			if( barheight>0 )
					parent_->drawFilledRectangle( x, y, barwidth, barheight );
			}
		}

	}
	graphpos_++;
    parent_->setStippleN(0);	//  Restore all-bits stipple.
    if ( dousedlegends_ )
    {
    	drawused( manditory );
    }
}
void FieldMeterGraph::checkResources( void )
{
  FieldMeterDecay::checkResources();

  const char *ptr = parent_->getResource( "graphNumCols" );
  if( ptr )
  {
    int i;
	if( sscanf( ptr, "%d", &i ) == 1 )
	{
		if( i>0 )
		{
			setNumCols( i );
		}
	}
  }
}
void FieldMeterGraph::setNumCols( int n )
{
	graphNumCols_ = n;
	graphpos_ = graphNumCols_-1;

	if( heightfield_ )
		delete [] heightfield_;
	heightfield_ = NULL;

}
