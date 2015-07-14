//
//  Copyright (c) 1994, 1995, 2004, 2006, 2015
//  Initial port performed by Stefan Eilemann (eilemann@gmail.com)
//
//  This file may be distributed under terms of the GPL
//
#include "memmeter.h"
#include "xosview.h"

#include <unistd.h>
#include <stdlib.h>
#include <invent.h>

MemMeter::MemMeter(XOSView *parent)
    : FieldMeterGraph(parent, 4, "MEM","KERNEL/FS/USER/FREE") {

    inventory_t *inv;
    _pageSize = getpagesize();
    minfosz = sysmp(MP_SASZ, MPSA_RMINFO);

    total_ = 0;
    setinvent();
    for( inv = getinvent(); inv != NULL; inv = getinvent() ) {
        if ( inv->inv_class==INV_MEMORY && inv->inv_type == INV_MAIN_MB ) {
            total_ = inv->inv_state*1024/_pageSize*1024;
            break;
        }
    }
    if ( total_==0 )
        logFatal << "can't find any memory." << std::endl;
}

void MemMeter::checkResources(void) {

    FieldMeterGraph::checkResources();

    setfieldcolor(0, parent_->getResource("memKernelColor"));
    setfieldcolor(1, parent_->getResource("memCacheColor"));
    setfieldcolor(2, parent_->getResource("memUsedColor"));
    setfieldcolor(3, parent_->getResource("memFreeColor"));
    priority_ = util::stoi (parent_->getResource("memPriority"));
    dodecay_ = parent_->isResourceTrue("memDecay");
    useGraph_ = parent_->isResourceTrue("memGraph");
    setUsedFormat(parent_->getResource("memUsedFormat"));
}

MemMeter::~MemMeter(void) {
}

void MemMeter::checkevent(void) {
    getmeminfo();
    drawfields(parent_->g());
}

void MemMeter::getmeminfo(void) {

    sysmp(MP_SAGET, MPSA_RMINFO, (char *) &mp, minfosz);

    fields_[0] = total_ - (mp.availrmem + mp.bufmem);

    fields_[1] = mp.bufmem +
        mp.dchunkpages + mp.dpages +
        mp.chunkpages - mp.dchunkpages;

    fields_[2] = mp.availrmem -
        (mp.freemem + mp.chunkpages + mp.dpages);

    fields_[3] = mp.freemem;

    FieldMeterDecay::setUsed( (fields_[0]+fields_[1]+fields_[2]) * _pageSize,
      total_ * _pageSize);
}
