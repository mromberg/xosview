//
//  Copyright (c) 1999, 2015
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//

#include "pagemeter.h"

#include <sys/sysinfo.h>


PageMeter::PageMeter(XOSView *parent, kstat_ctl_t *_kc, float max)
    : FieldMeterGraph( parent, 3, "PAGE", "IN/OUT/IDLE"),
      pageinfo_(2, std::vector<float>(2, 0.0)),
      pageindex_(0),
      maxspeed_(max),
      cpustats(KStatList::getList(_kc, KStatList::CPU_STAT)),
      kc(_kc) {
}


PageMeter::~PageMeter(void) {
}


void PageMeter::checkResources(void) {

    FieldMeterGraph::checkResources();

    setfieldcolor(0, parent_->getResource("pageInColor"));
    setfieldcolor(1, parent_->getResource("pageOutColor"));
    setfieldcolor(2, parent_->getResource("pageIdleColor"));
    priority_ = util::stoi(parent_->getResource("pagePriority"));
    maxspeed_ *= priority_ / 10.0;
    dodecay_ = parent_->isResourceTrue("pageDecay");
    useGraph_ = parent_->isResourceTrue("pageGraph");
    setUsedFormat(parent_->getResource("pageUsedFormat"));
    decayUsed(parent_->isResourceTrue("pageUsedDecay"));
}


void PageMeter::checkevent(void) {
    getpageinfo();
    drawfields(parent_->g());
}


void PageMeter::getpageinfo(void) {
    cpu_stat_t cs;
    total_ = 0;
    pageinfo_[pageindex_][0] = 0; // pgin
    pageinfo_[pageindex_][1] = 0; // pgout
    cpustats->update(kc);

    for (unsigned int i = 0; i < cpustats->count(); i++) {
        if (kstat_read(kc, (*cpustats)[i], &cs) == -1)
            continue;
        pageinfo_[pageindex_][0] += cs.cpu_vminfo.pgpgin;
        pageinfo_[pageindex_][1] += cs.cpu_vminfo.pgpgout;
    }

    int oldindex = (pageindex_ + 1) % 2;
    for (int i = 0; i < 2; i++) {
        if (pageinfo_[oldindex][i] == 0)
            pageinfo_[oldindex][i] = pageinfo_[pageindex_][i];

        fields_[i] = pageinfo_[pageindex_][i] - pageinfo_[oldindex][i];
        total_ += fields_[i];
    }

    if (total_ > maxspeed_)
        fields_[2] = 0.0;
    else {
        fields_[2] = maxspeed_ - total_;
        total_ = maxspeed_;
    }

    setUsed(total_ - fields_[2], maxspeed_);
    pageindex_ = (pageindex_ + 1) % 2;
}
