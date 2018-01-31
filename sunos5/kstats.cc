//
//  Copyright (c) 2015, 2018
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//
#include "kstats.h"


KStatList::KStatList(kstat_ctl_t *kcp, module m)
    : _chain(kcp->kc_chain_id), _m(m) {

    getstats(kcp);
}


void KStatList::getstats(kstat_ctl_t *kcp) {
    for (kstat_t *ksp = kcp->kc_chain; ksp != nullptr; ksp = ksp->ks_next) {
        std::string ks_name(ksp->ks_name, 0, 8);
        if (_m == CPU_STAT && ks_name == "cpu_stat")
            _stats.push_back(ksp);
        if (_m == CPU_INFO && ks_name == "cpu_info")
            _stats.push_back(ksp);
        if (_m == CPU_SYS && ksp->ks_type == KSTAT_TYPE_NAMED &&
          std::string(ksp->ks_module, 0, 3) == "cpu" &&
          ks_name.substr(0, 3) == "sys")
            _stats.push_back(ksp);
        if (_m == DISKS && ksp->ks_type == KSTAT_TYPE_IO &&
          std::string(ksp->ks_class, 0, 4) == "disk")
            _stats.push_back(ksp);
        if (_m == NETS && ksp->ks_type == KSTAT_TYPE_NAMED &&
          std::string(ksp->ks_class, 0, 3) == "net" &&
          ( std::string(ksp->ks_module, 0, 4) == "link" ||
            std::string(ksp->ks_module, 0, 2) == "lo" ))
            _stats.push_back(ksp);
    }
}


KStatList *KStatList::getList(kstat_ctl_t *kcp, module m) {
    switch (m) {
    case CPU_STAT:
        static KStatList cpu_stats(kcp, m);
        return &cpu_stats;
    case CPU_INFO:
        static KStatList cpu_infos(kcp, m);
        return &cpu_infos;
    case CPU_SYS:
        static KStatList cpu_sys(kcp, m);
        return &cpu_sys;
    case DISKS:
        static KStatList disks(kcp, m);
        return &disks;
    case NETS:
        static KStatList nets(kcp, m);
        return &nets;
    default:
        return nullptr;
    }
}


void KStatList::update(kstat_ctl_t *kcp) {
    if (kstat_chain_update(kcp) > 0 || _chain != kcp->kc_chain_id) {
        logDebug << "kstat chain id changed to "
                 << kcp->kc_chain_id << std::endl;
        _chain = kcp->kc_chain_id;
        _stats.clear();
        getstats(kcp);
    }
}
