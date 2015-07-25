//
//  Copyright (c) 1999, 2015
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//

#include "memmeter.h"
#include "kstats.h"




MemMeter::MemMeter(XOSView *parent, kstat_ctl_t *_kc)
    : FieldMeterGraph(parent, 4, "MEM", "SYS/ZFS/OTHER/FREE"),
      pageSize(sysconf(_SC_PAGESIZE)),
      kc(_kc),
      ksp_sp(0), ksp_zfs(0) {

    total_ = sysconf(_SC_PHYS_PAGES);

    ksp_sp = kstat_lookup(kc, const_cast<char *>("unix"), 0,
      const_cast<char *>("system_pages"));
    ksp_zfs = kstat_lookup(kc, const_cast<char *>("vmem"), -1,
      const_cast<char *>("zfs_file_data_buf"));
    if (ksp_sp == NULL) { // ZFS cache may be missing
        logFatal << "Can not find unix:0:system_pages kstat." << std::endl;
    }
}


void MemMeter::checkResources(void) {

    FieldMeterGraph::checkResources();

    setfieldcolor(0, parent_->getResource("memKernelColor"));
    setfieldcolor(1, parent_->getResource("memCacheColor"));
    setfieldcolor(2, parent_->getResource("memUsedColor"));
    setfieldcolor(3, parent_->getResource("memFreeColor"));
    priority_ = util::stoi(parent_->getResource("memPriority"));
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

    fields_[1] = 0;
    if (ksp_zfs) {
        if (kstat_read(kc, ksp_zfs, NULL) == -1)
            logFatal << "Can't read vmem::zfs_file_data_buf kstat." << std::endl;

        k = (kstat_named_t *)kstat_data_lookup(ksp_zfs,
          const_cast<char *>("mem_inuse"));
        if (k == NULL) {
            logFatal << "Can't read "
                     << "vmem::zfs_file_data_buf:mem_inuse kstat."
                     << std::endl;
        }
        fields_[1] = kstat_to_double(k) / pageSize;
    }

    if (kstat_read(kc, ksp_sp, NULL) == -1)
        logFatal << "Can not read unix:0:system_pages kstat." << std::endl;

    k = (kstat_named_t *)kstat_data_lookup(ksp_sp,
      const_cast<char *>("pp_kernel"));
    if (k == NULL) {
        logFatal << "Can not read "
                 << "unix:0:system_pages:pp_kernel kstat." << std::endl;
    }

    fields_[0] = kstat_to_double(k) - fields_[1];
    k = (kstat_named_t *)kstat_data_lookup(ksp_sp,
      const_cast<char *>("freemem"));
    if (k == NULL) {
        logFatal << "Can not read "
                 << "unix:0:system_pages:freemem kstat." << std::endl;
    }
    fields_[3] = kstat_to_double(k);
    fields_[2] = total_ - (fields_[0] + fields_[1] + fields_[3]);
    logDebug << "kernel: "
             << (unsigned long long)(fields_[0] * pageSize / 1024) << " kB "
             << "zfs: "
             << (unsigned long long)(fields_[1] * pageSize / 1024) <<" kB "
             << "other: "
             << (unsigned long long)(fields_[2] * pageSize / 1024) << " kB "
             << "free: "
             << (unsigned long long)(fields_[3] * pageSize / 1024) << " kB"
             << std::endl;

    setUsed((fields_[0] + fields_[1] + fields_[2]) * pageSize,
      total_ * pageSize);
}
