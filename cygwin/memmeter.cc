//
//  Copyright (c) 1994, 1995, 2006, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "memmeter.h"



MemMeter::MemMeter(void)
    : PrcMemMeter(2, "USED/FREE") {
}


void MemMeter::checkResources(const ResDB &rdb) {
    FieldMeterGraph::checkResources(rdb);

    setfieldcolor(0, rdb.getColor( "memUsedColor"));
    setfieldcolor(1, rdb.getColor( "memFreeColor"));
}


std::vector<PrcMemMeter::LineInfo> MemMeter::newLineInfos(void) {
    std::vector<LineInfo> infos = {
        LineInfo("MemTotal", &_total),
        LineInfo("MemFree", &_fields[1])
    };

    return infos;
}


void MemMeter::setFields(void) {
    _fields[0] = _total - _fields[1];

    if (_total)
        FieldMeterDecay::setUsed(_total - _fields[1], _total);
}
