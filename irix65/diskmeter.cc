//  
// $Id: diskmeter.cc,v 1.2 2003/04/24 06:15:19 eile Exp $
//  Initial port performed by Stefan Eilemann (eile@sgi.com)
//

#include "diskmeter.h"
#include "xosview.h"
#include "sarmeter.h"

#include <stdlib.h>


DiskMeter::DiskMeter( XOSView *parent, float max ) : FieldMeterGraph(
    parent, 3, "DISK", "READ/WRITE/IDLE")
{
    maxspeed_ = max;
    getdiskinfo();
}

DiskMeter::~DiskMeter( void )
{
}

void DiskMeter::checkResources( void )
{
    FieldMeterGraph::checkResources();
    
    setfieldcolor( 0, parent_->getResource("diskReadColor") );
    setfieldcolor( 1, parent_->getResource("diskWriteColor") );
    setfieldcolor( 2, parent_->getResource("diskIdleColor") );
    priority_ = atoi (parent_->getResource( "diskPriority" ) );
    dodecay_ = parent_->isResourceTrue("diskDecay" );
    useGraph_ = parent_->isResourceTrue( "diskGraph" );
    SetUsedFormat(parent_->getResource("diskUsedFormat"));
}

void DiskMeter::checkevent( void )
{
    getdiskinfo();
    drawfields();
}

void DiskMeter::getdiskinfo( void )
{
    SarMeter::DiskInfo *di = SarMeter::Instance()->getDiskInfo();

    // new data
    total_ = maxspeed_;

#if 0
    // fix overflow (conversion bug?)
    if (fields_[0] < 0.0)
        fields_[0] = 0.0;
    if (fields_[1] < 0.0)
        fields_[1] = 0.0;
#endif
    
    if (fields_[0] + fields_[1] > total_)
       	total_ = fields_[0] + fields_[1];

    fields_[2] = total_ - (fields_[0] + fields_[1]);
    
    setUsed((fields_[0]+fields_[1]), total_);
    IntervalTimerStart();
}

