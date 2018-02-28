//
//  Copyright (c) 1999, 2015, 2018
//  Rewritten for Solaris by Arno Augustin 1999
//  augustin@informatik.uni-erlangen.de
//
//  This file may be distributed under terms of the GPL
//

#include "diskmeter.h"



DiskMeter::DiskMeter( kstat_ctl_t *kc)
    : ComDiskMeter(),
      _read_prev(0), _write_prev(0),
      _kc(kc),
      _disks(KStatList::getList(_kc, KStatList::DISKS)) {
}


std::pair<double, double> DiskMeter::getRate(void) {
    kstat_io_t kio;
    uint64_t read_curr = 0, write_curr = 0;
    _disks->update(_kc);

    timerStop();
    for (unsigned int i = 0; i < _disks->count(); i++) {
        if ( kstat_read(_kc, (*_disks)[i], &kio) == -1 )
            continue;
        logDebug << (*_disks)[i]->ks_name << ": "
                 << kio.nread << " bytes read "
                 << kio.nwritten << " bytes written." << std::endl;
        read_curr += kio.nread;
        write_curr += kio.nwritten;
    }
    if (_read_prev == 0)
        _read_prev = read_curr;
    if (_write_prev == 0)
        _write_prev = write_curr;

    double t = etimeSecs();
    std::pair<double, double> rval(0, 0);
    rval.first = (double)(read_curr - _read_prev) / t;
    rval.second = (double)(write_curr - _write_prev) / t;

    timerStart();
    _read_prev = read_curr;
    _write_prev = write_curr;

    return rval;
}
