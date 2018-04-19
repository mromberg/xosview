//
//  Copyright (c) 1994, 1995, 2006, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "pmemmeter.h"

#include <fstream>
#include <iomanip>


static const char * const MEMFILENAME = "/proc/meminfo";



PrcMemMeter::PrcMemMeter(size_t numfields, const std::string &legend)
    : FieldMeterGraph(numfields, "MEM", legend) {
}


void PrcMemMeter::checkevent(void) {
    if (_lineInfos.empty())
        _lineInfos = findLines(newLineInfos(), MEMFILENAME);

    getmemstat(MEMFILENAME, _lineInfos);
    setFields();

#ifdef XOSVDEBUG
    // for debugging.
    const float TOMEG = 1.0 / (1 << 20);
    logDebug << std::setprecision(1) << std::fixed
             << "--- memmeter---" << std::endl
             << "_total " << _total * TOMEG << "M" << std::endl;
    for (const auto &li : _lineInfos)
        logDebug << std::setprecision(1) << std::fixed
                 << li.id() << " " << li.val() * TOMEG << "M" << std::endl;
#endif
}


std::vector<PrcMemMeter::LineInfo> PrcMemMeter::findLines(
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

        for (const auto &li : tmplate)
            if (li.id() == buf.substr(0, li.id().size())) {
                logDebug << "FOUND line: " << buf << std::endl;
                rval[inum] = li;
                rval[inum].line(lineNum);
                inum++;
            }
    }

    return rval;
}


void PrcMemMeter::getmemstat(const std::string &fname,
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
        for (auto &li : infos)
            if (li.line() == lineNum) {
                std::istringstream line(buf);
                size_t val;
                std::string ignore;
                line >> ignore >> val;
                // All stats are in KB.
                // Multiply by 1024 bytes per K
                li.setVal(val * 1024.0);
                fcount++;
                break;
            }

        if (fcount >= infos.size())
            break;
    }
}
