//
//  Copyright (c) 1999, 2015, 2018
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//

#include "memmeter.h"
#include "kstats.h"

#include <unistd.h>



MemMeter::MemMeter(kstat_ctl_t *kc)
    : FieldMeterGraph(4, "MEM", "SYS/ZFS/OTHER/FREE"),
      _pageSize(sysconf(_SC_PAGESIZE)),
      _kc(kc), _ksp_sp(nullptr), _ksp_zfs(nullptr) {

    _total = sysconf(_SC_PHYS_PAGES);

    _ksp_sp = kstat_lookup(kc, const_cast<char *>("unix"), 0,
      const_cast<char *>("system_pages"));
    _ksp_zfs = kstat_lookup(kc, const_cast<char *>("vmem"), -1,
      const_cast<char *>("zfs_file_data_buf"));
    if (_ksp_sp == nullptr) { // ZFS cache may be missing
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

    kstat_named_t *k = nullptr;
    _fields[1] = 0;

    if (_ksp_zfs) {
        if (kstat_read(_kc, _ksp_zfs, nullptr) == -1)
            logFatal << "Can't read vmem::zfs_file_data_buf kstat."
                     << std::endl;

        if ((k = KStatList::lookup(_ksp_zfs, "mem_inuse")) == nullptr) {
            logFatal << "Can't read "
                     << "vmem::zfs_file_data_buf:mem_inuse kstat."
                     << std::endl;
        }
        _fields[1] = kstat_to_double(k) / _pageSize;
    }

    if (kstat_read(_kc, _ksp_sp, nullptr) == -1)
        logFatal << "Can not read unix:0:system_pages kstat." << std::endl;

    if ((k = KStatList::lookup(_ksp_sp, "pp_kernel")) == nullptr) {
        logFatal << "Can not read "
                 << "unix:0:system_pages:pp_kernel kstat." << std::endl;
    }
    _fields[0] = kstat_to_double(k) - _fields[1];

    if ((k = KStatList::lookup(_ksp_sp, "freemem")) == nullptr) {
        logFatal << "Can not read "
                 << "unix:0:system_pages:freemem kstat." << std::endl;
    }
    _fields[3] = kstat_to_double(k);
    _fields[2] = _total - (_fields[0] + _fields[1] + _fields[3]);

    logDebug << "kernel: "
             << static_cast<uint64_t>(_fields[0] * _pageSize / 1024) << " kB "
             << "zfs: "
             << static_cast<uint64_t>(_fields[1] * _pageSize / 1024) <<" kB "
             << "other: "
             << static_cast<uint64_t>(_fields[2] * _pageSize / 1024) << " kB "
             << "free: "
             << static_cast<uint64_t>(_fields[3] * _pageSize / 1024) << " kB"
             << std::endl;

    setUsed((_fields[0] + _fields[1] + _fields[2]) * _pageSize,
      _total * _pageSize);
}
