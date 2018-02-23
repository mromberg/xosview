//
//  Copyright (c) 1994, 1995, 2006, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "memmeter.h"

#include <fstream>
#include <iomanip>


static const char * const MEMFILENAME = "/proc/meminfo";



MemMeter::MemMeter(void)
    : FieldMeterGraph(2, "MEM", "USED/FREE") {

    initLineInfo();
}


void MemMeter::checkResources(const ResDB &rdb) {
    FieldMeterGraph::checkResources(rdb);

    setfieldcolor(0, rdb.getColor( "memUsedColor"));
    setfieldcolor(1, rdb.getColor( "memFreeColor"));
}


void MemMeter::checkevent(void) {
    getmeminfo();
    /* for debugging (see below) */
    const float TOMEG = 1.0 / 1024.0 / 1024.0;
    logDebug << std::setprecision(1) << std::fixed
             << "t " << _total * TOMEG << " "
             << "used "    << _fields[0] * TOMEG << " "
//             << "buf "     << _fields[1] * TOMEG << " "
//             << "cache "   << _fields[2] * TOMEG << " "
//             << "swcache " << _fields[3] * TOMEG << " "
             << "free "    << _fields[1] * TOMEG
             << std::endl;
}


void MemMeter::getmeminfo(void) {
    getmemstat(MEMFILENAME, _MIlineInfos);

    _fields[0] = _total - _fields[1];

    if (_total)
        FieldMeterDecay::setUsed(_total - _fields[1], _total);
}


std::vector<MemMeter::LineInfo> MemMeter::findLines(
    const std::vector<LineInfo> &tmplate, const std::string &fname) const {

    std::ifstream meminfo(fname);
    if (!meminfo) {
        logFatal << "Can not open file : " << fname << std::endl;
    }

    std::vector<LineInfo> rval(tmplate.size());

    // Get the info from the "standard" meminfo file.
    size_t lineNum = 0;
    size_t inum = 0;  // which info are we going to insert
    while (!meminfo.eof()) {
        std::string buf;
        std::getline(meminfo, buf);
        lineNum++;

        for (const auto &t : tmplate)
            if (t.id() == buf.substr(0, t.id().size())) {
                logDebug << "FOUND line: " << buf << std::endl;
                rval[inum] = t;
                rval[inum].line(lineNum);
                inum++;
            }
    }

    return rval;
}


void MemMeter::initLineInfo(void) {
    std::vector<LineInfo> infos = { LineInfo("MemTotal", &_total),
                                    LineInfo("MemFree", &_fields[1]) };

    _MIlineInfos = findLines(infos, MEMFILENAME);
}


void MemMeter::getmemstat(const std::string &fname,
  std::vector<LineInfo> &infos) const {

    std::ifstream meminfo(fname);
    if (!meminfo) {
        logFatal << "Can not open file : " << fname << std::endl;
    }

    // Get the info from the "standard" meminfo file.
    size_t lineNum = 0;
    size_t fcount = 0; // found count
    while (!meminfo.eof()) {
        std::string buf;
        std::getline(meminfo, buf);
        lineNum++;
        for (auto &info : infos)
            if (info.line() == lineNum) {
                std::istringstream line(buf);
                unsigned long val;
                std::string ignore;
                line >> ignore >> val;
                // All stats are in KB.
                // Multiply by 1024 bytes per K
                info.setVal(val * 1024.0);
                fcount++;
                break;
            }

        if (fcount >= infos.size())
            break;
    }
}
