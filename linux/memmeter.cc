//
//  Copyright (c) 1994, 1995, 2006, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "memmeter.h"



MemMeter::MemMeter(void)
    : PrcMemMeter(5, "USED/BUFF/CACHE/SCACHE/FREE") {
}


void MemMeter::checkResources(const ResDB &rdb) {
    FieldMeterGraph::checkResources(rdb);

    setfieldcolor(0, rdb.getColor( "memUsedColor"));
    setfieldcolor(1, rdb.getColor( "memBufferColor"));
    setfieldcolor(2, rdb.getColor( "memCacheColor"));
    setfieldcolor(3, rdb.getColor( "memSwapCacheColor"));
    setfieldcolor(4, rdb.getColor( "memFreeColor"));
}


std::vector<PrcMemMeter::LineInfo> MemMeter::newLineInfos(void) {
    std::vector<LineInfo> infos = {
        LineInfo("MemTotal", &_total),
        LineInfo("Buffers", &_fields[1]),
        LineInfo("Cached", &_fields[2]),
        LineInfo("SwapCached", &_fields[3]),
        LineInfo("MemFree", &_fields[4])
    };

    return infos;
}


void MemMeter::setFields(void) {
    _fields[0] = _total - _fields[4] - _fields[3] - _fields[2] - _fields[1];

    if (_total)
        setUsed(_total - _fields[3] - _fields[4], _total);
}
