//
//  Copyright (c) 1999, 2015, 2018
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//

#include "memmeter.h"
#include "kstats.h"

#include <unistd.h>



MemMeter::MemMeter(kstat_ctl_t *_kc)
    : FieldMeterGraph(4, "MEM", "SYS/ZFS/OTHER/FREE"),
      pageSize(sysconf(_SC_PAGESIZE)),
      kc(_kc),
      ksp_sp(0), ksp_zfs(0) {

    _total = sysconf(_SC_PHYS_PAGES);

    ksp_sp = kstat_lookup(kc, const_cast<char *>("unix"), 0,
      const_cast<char *>("system_pages"));
    ksp_zfs = kstat_lookup(kc, const_cast<char *>("vmem"), -1,
      const_cast<char *>("zfs_file_data_buf"));
    if (ksp_sp == nullptr) { // ZFS cache may be missing
        logFatal << "Can not find unix:0:system_pages kstat." << std::endl;
    }
}


void MemMeter::checkResources(const ResDB &rdb) {

    FieldMeterGraph::checkResources(rdb);

    setfieldcolor(0, rdb.getColor("memKernelColor"));
    setfieldcolor(1, rdb.getColor("memCacheColor"));
    setfieldcolor(2, rdb.getColor("memUsedColor"));
    setfieldcolor(3, rdb.getColor("memFreeColor"));
}


void MemMeter::checkevent(void) {
    getmeminfo();
}


void MemMeter::getmeminfo(void) {
    kstat_named_t *k;

    _fields[1] = 0;
    if (ksp_zfs) {
        if (kstat_read(kc, ksp_zfs, nullptr) == -1)
            logFatal << "Can't read vmem::zfs_file_data_buf kstat."
                     << std::endl;

        k = (kstat_named_t *)kstat_data_lookup(ksp_zfs,
          const_cast<char *>("mem_inuse"));
        if (k == nullptr) {
            logFatal << "Can't read "
                     << "vmem::zfs_file_data_buf:mem_inuse kstat."
                     << std::endl;
        }
        _fields[1] = kstat_to_double(k) / pageSize;
    }

    if (kstat_read(kc, ksp_sp, nullptr) == -1)
        logFatal << "Can not read unix:0:system_pages kstat." << std::endl;

    k = (kstat_named_t *)kstat_data_lookup(ksp_sp,
      const_cast<char *>("pp_kernel"));
    if (k == nullptr) {
        logFatal << "Can not read "
                 << "unix:0:system_pages:pp_kernel kstat." << std::endl;
    }

    _fields[0] = kstat_to_double(k) - _fields[1];
    k = (kstat_named_t *)kstat_data_lookup(ksp_sp,
      const_cast<char *>("freemem"));
    if (k == nullptr) {
        logFatal << "Can not read "
                 << "unix:0:system_pages:freemem kstat." << std::endl;
    }
    _fields[3] = kstat_to_double(k);
    _fields[2] = _total - (_fields[0] + _fields[1] + _fields[3]);
    logDebug << "kernel: "
             << (unsigned long long)(_fields[0] * pageSize / 1024) << " kB "
             << "zfs: "
             << (unsigned long long)(_fields[1] * pageSize / 1024) <<" kB "
             << "other: "
             << (unsigned long long)(_fields[2] * pageSize / 1024) << " kB "
             << "free: "
             << (unsigned long long)(_fields[3] * pageSize / 1024) << " kB"
             << std::endl;

    setUsed((_fields[0] + _fields[1] + _fields[2]) * pageSize,
      _total * pageSize);
}
