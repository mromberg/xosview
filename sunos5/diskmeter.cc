//
//  Copyright (c) 1999, 2015
//  Rewritten for Solaris by Arno Augustin 1999
//  augustin@informatik.uni-erlangen.de
//
//  This file may be distributed under terms of the GPL
//

#include "diskmeter.h"



DiskMeter::DiskMeter( XOSView *parent, kstat_ctl_t *kc, float max )
    : FieldMeterGraph( parent, 3, "DISK", "READ/WRITE/IDLE" ),
      _read_prev(0), _write_prev(0),
      _maxspeed(max),
      _kc(kc),
      _disks(KStatList::getList(_kc, KStatList::DISKS)) {
}


DiskMeter::~DiskMeter( void ) {
}


void DiskMeter::checkResources(const ResDB &rdb) {

    FieldMeterGraph::checkResources(rdb);

    setfieldcolor( 0, rdb.getResource("diskReadColor") );
    setfieldcolor( 1, rdb.getResource("diskWriteColor") );
    setfieldcolor( 2, rdb.getResource("diskIdleColor") );
    priority_ = util::stoi( rdb.getResource("diskPriority") );
    dodecay_ = rdb.isResourceTrue("diskDecay");
    useGraph_ = rdb.isResourceTrue("diskGraph");
    setUsedFormat( rdb.getResource("diskUsedFormat") );
    decayUsed(rdb.isResourceTrue("diskUsedDecay"));
}


void DiskMeter::checkevent( void ) {
    getdiskinfo();
    drawfields(parent_->g());
}


void DiskMeter::getdiskinfo( void ) {
    total_ = _maxspeed;
    kstat_io_t kio;
    uint64_t read_curr = 0, write_curr = 0;
    _disks->update(_kc);

    IntervalTimerStop();
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

    double t = IntervalTimeInSecs();
    fields_[0] = (double)(read_curr - _read_prev) / t;
    fields_[1] = (double)(write_curr - _write_prev) / t;

    IntervalTimerStart();
    _read_prev = read_curr;
    _write_prev = write_curr;

    if (fields_[0] < 0)
        fields_[0] = 0;
    if (fields_[1] < 0)
        fields_[1] = 0;
    if (fields_[0] + fields_[1] > total_)
        total_ = fields_[0] + fields_[1];
    fields_[2] = total_ - (fields_[0] + fields_[1]);
    setUsed(fields_[0] + fields_[1], total_);
}
