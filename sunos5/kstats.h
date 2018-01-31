//
//  Copyright (c) 2015, 2018
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//
#ifndef kstats_h
#define kstats_h

#include "log.h"

#include <vector>

#include <kstat.h>



// Helper to keep track of kstats.
class KStatList {
public:
    KStatList(const KStatList &) = delete;
    KStatList &operator=(const KStatList &) = delete;

    enum module {  // module:instance:name (class)
        CPU_STAT,    // *:*:cpu_stat*
        CPU_INFO,    // *:*:cpu_info*
        CPU_SYS,     // cpu:*:sys
        DISKS,       // *:*:*         (disk)
        NETS         // {link,lo}:*:* (net)
    };
    static KStatList *getList(kstat_ctl_t *kcp, module m);

    kstat_t *operator[](size_t i)
        { return i < _stats.size() ? _stats[i] : nullptr; }

    size_t count(void) { return _stats.size(); }
    void update(kstat_ctl_t *kcp);

private:
    kid_t _chain;
    module _m;
    std::vector<kstat_t *> _stats;

    KStatList(kstat_ctl_t *kcp, module m);
    void getstats(kstat_ctl_t *kcp);
};


// Read the correct value from "named" type kstat.
inline double kstat_to_double(kstat_named_t *k) {
    switch (k->data_type) {
    case KSTAT_DATA_INT32:
        return k->value.i32;
    case KSTAT_DATA_UINT32:
        return k->value.ui32;
#if defined(_INT64_TYPE)
    case KSTAT_DATA_INT64:
        return k->value.i64;
    case KSTAT_DATA_UINT64:
        return k->value.ui64;
#endif
    case KSTAT_DATA_FLOAT:
        return k->value.f;
    case KSTAT_DATA_DOUBLE:
        return k->value.d;
    case KSTAT_DATA_CHAR:
    case KSTAT_DATA_STRING:
    default:
        logFatal << "kstat data type " << k->data_type
                 << " can not be converted to number."
                 << std::endl;
    }

    return 0.0;
}


inline unsigned long long kstat_to_ui64(kstat_named_t *k) {
    switch (k->data_type) {
    case KSTAT_DATA_INT32:
        return k->value.i32;
    case KSTAT_DATA_UINT32:
        return k->value.ui32;
#if defined(_INT64_TYPE)
    case KSTAT_DATA_INT64:
        return k->value.i64;
    case KSTAT_DATA_UINT64:
        return k->value.ui64;
#endif
    case KSTAT_DATA_FLOAT:
        return k->value.f;
    case KSTAT_DATA_DOUBLE:
        return k->value.d;
    case KSTAT_DATA_CHAR:
    case KSTAT_DATA_STRING:
    default:
        logFatal << "kstat data type " << k->data_type
                 << " can not be converted to number."
                 << std::endl;
    }

    return 0;
}


#endif
