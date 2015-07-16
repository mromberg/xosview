//
//  Copyright (c) 2015
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//
#include "swapmeter.h"

#include <unistd.h>
#include <sys/swap.h>


SwapMeter::SwapMeter(XOSView *parent)
    : FieldMeterGraph(parent, 3, "SWAP", "USED/RSVD/FREE") {
}

SwapMeter::~SwapMeter(void) {
}

void SwapMeter::checkResources(void) {

    FieldMeterGraph::checkResources();

    setfieldcolor(0, parent_->getResource("swapUsedColor"));
    setfieldcolor(1, parent_->getResource("swapReservedColor"));
    setfieldcolor(2, parent_->getResource("swapFreeColor"));
    priority_ = util::stoi(parent_->getResource("swapPriority"));
    dodecay_ = parent_->isResourceTrue("swapDecay");
    useGraph_ = parent_->isResourceTrue("swapGraph");
    setUsedFormat(parent_->getResource("swapUsedFormat"));
}

void SwapMeter::checkevent(void) {
    getswapinfo();
    drawfields(parent_->g());
}

void SwapMeter::getswapinfo(void) {

    struct anoninfo ai;

    if (swapctl(SC_AINFO, &ai) == -1)
        return;

    total_ = ai.ani_max;
    fields_[0] = (ai.ani_max - ai.ani_free); // allocated
    fields_[1] = (ai.ani_resv - (ai.ani_max - ai.ani_free)); // reserved
    fields_[2] = (ai.ani_max - ai.ani_resv); // available

    setUsed((fields_[0] + fields_[1]) * pageSize(), total_ * pageSize());
}

size_t SwapMeter::pageSize(void) {
    static size_t ps = 0;
    static bool first = true;
    if (first) {
        first = false;
        ps = sysconf(_SC_PAGESIZE);
    }

    return ps;
}
