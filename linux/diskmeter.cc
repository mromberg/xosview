//  
//  Copyright (c) 1999 by Mike Romberg (romberg@fsl.noaa.gov)
//
//  This file may be distributed under terms of the GPL
//
// $Id: diskmeter.cc,v 1.3 1999/01/31 12:54:30 mcnab Exp $
//

#include "diskmeter.h"
#include "xosview.h"
#include <fstream.h>
#include <stdlib.h>

static const char STATFILENAME[] = "/proc/stat";

DiskMeter::DiskMeter( XOSView *parent, float max ) : FieldMeterGraph(
  parent, 2, "DISK", "XFER/IDLE")
    {
    prev_ = 0;
    maxspeed_ = max;
    }

DiskMeter::~DiskMeter( void )
    {
    }

void DiskMeter::checkResources( void )
    {
    FieldMeterDecay::checkResources();

    setfieldcolor( 0, parent_->getResource("diskUsedColor") );
    setfieldcolor( 1, parent_->getResource("diskIdleColor") );
    priority_ = atoi (parent_->getResource( "diskPriority" ) );
    dodecay_ = !strncasecmp(parent_->getResource("diskDecay" ), "True", 5);
    useGraph_ = !strncasecmp(parent_->getResource( "diskGraph" ), "True", 5 );
    SetUsedFormat(parent_->getResource("diskUsedFormat"));
    }

void DiskMeter::checkevent( void )
    {
    getdiskinfo();
    drawfields();
    }

void DiskMeter::getdiskinfo( void )
    {
    IntervalTimerStop();
    total_ = maxspeed_;
    char buf[256];
    ifstream stats( STATFILENAME );

    if ( !stats )
        {
        cerr <<"Can not open file : " <<STATFILENAME <<endl;
        exit( 1 );
        }

    // Find the line which starts with "disk"
    stats >> buf;
    while (strncmp(buf, "disk", 4))
        {
        stats.ignore(1024, '\n');
        stats >> buf;
        }

    unsigned long one, two, three, four;
    stats >>one >> two >> three >> four;
    // assume each "unit" is 4k.  This could very well be wrong.
    unsigned long int curr = (one + two + three + four) * 4 * 1024;
    if( prev_ )
    {
    	fields_[0] = (curr - prev_) / 1.0;
    	if (fields_[0] > total_)
        	total_ = fields_[0];
    	fields_[1] = total_ - fields_[0];
    }
    else 
    {
		fields_[0] = 0;
		fields_[1] = 0;
    }

    prev_ = curr;

    setUsed(fields_[0] * IntervalTimeInMicrosecs()/1e6, total_);
    IntervalTimerStart();
    }
