//
//  Copyright (c) 2015
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//
#include "memmeter.h"
#include "xosview.h"

#include <unistd.h>
#include <stdlib.h>


MemMeter::MemMeter(XOSView *parent, kstat_ctl_t *_kc)
    : FieldMeterGraph(parent, 2, "MEM", "USED/FREE") {

    kc = _kc;

    _pageSize = sysconf(_SC_PAGESIZE);
    total_ = sysconf(_SC_PHYS_PAGES);

    std::string ustr("unix"), spstr("system_pages");
    ksp = kstat_lookup(kc, const_cast<char *>(ustr.c_str()), 0,
      const_cast<char *>(spstr.c_str()));
    if (ksp == NULL)
        logFatal << "kstat_lookup() failed" << std::endl;
}

void MemMeter::checkResources(void) {

    FieldMeterGraph::checkResources();

    setfieldcolor(0, parent_->getResource("memUsedColor"));
    setfieldcolor(1, parent_->getResource("memFreeColor"));
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
    kstat_named_t *k;

    if (kstat_read(kc, ksp, NULL) == -1)
        logFatal << "kstat_read() failed." << std::endl;

    std::string fmstr("freemem");
    k = (kstat_named_t *)kstat_data_lookup(ksp,
      const_cast<char *>(fmstr.c_str()));
    if (k == NULL)
        logFatal << "kstat_data_lookup() failed." << std::endl;

    fields_[0] = total_ - k->value.l;
    fields_[1] = k->value.l;

    FieldMeterDecay::setUsed(fields_[0] * _pageSize, total_ * _pageSize);
}
