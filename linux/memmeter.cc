//
//  Copyright (c) 1994, 1995, 2006, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "memmeter.h"

#include <fstream>
#include <sstream>
#include <iomanip>


static const char * const MEMFILENAME = "/proc/meminfo";



MemMeter::MemMeter( void )
    : FieldMeterGraph( 5, "MEM", "USED/BUFF/CACHE/SCACHE/FREE" ) {

    initLineInfo();
}


MemMeter::~MemMeter( void ){
}


void MemMeter::checkResources(const ResDB &rdb){
    FieldMeterGraph::checkResources(rdb);

    setfieldcolor( 0, rdb.getColor( "memUsedColor" ) );
    setfieldcolor( 1, rdb.getColor( "memBufferColor" ) );
    setfieldcolor( 2, rdb.getColor( "memCacheColor" ) );
    setfieldcolor( 3, rdb.getColor( "memSwapCacheColor") );
    setfieldcolor( 4, rdb.getColor( "memFreeColor" ) );
}


void MemMeter::checkevent( void ){
    getmeminfo();
    /* for debugging (see below) */
    const float TOMEG = 1.0/1024.0/1024.0;
    logDebug << std::setprecision(1) << std::fixed
             << "t " << _total * TOMEG << " "
             << "used "    << _fields[0] * TOMEG << " "
             << "buf "     << _fields[1] * TOMEG << " "
             << "cache "   << _fields[2] * TOMEG << " "
             << "swcache " << _fields[3] * TOMEG << " "
             << "free "    << _fields[4] * TOMEG
             << std::endl;
}

// FIXME: /proc/memstat and /proc/meminfo don't seem to correspond
// maybe it is time to fix this in the kernel and get real infos ...

void MemMeter::getmeminfo( void ){
    getmemstat(MEMFILENAME, _MIlineInfos);

    _fields[0] = _total - _fields[4] - _fields[3] - _fields[2] - _fields[1];

    if (_total)
        FieldMeterDecay::setUsed (_total - _fields[3] - _fields[4], _total);
}


std::vector<MemMeter::LineInfo> MemMeter::findLines(
    const std::vector<LineInfo> &tmplate, const std::string &fname){
    std::ifstream meminfo(fname.c_str());
    if (!meminfo){
        logFatal << "Can not open file : " << fname << std::endl;
    }

    std::vector<LineInfo> rval(tmplate.size());

    std::string buf;

    // Get the info from the "standard" meminfo file.
    int lineNum = 0;
    int inum = 0;  // which info are we going to insert
    while (!meminfo.eof()){
        std::getline(meminfo, buf);
        lineNum++;

        for (size_t i = 0 ; i < tmplate.size() ; i++)
            if (tmplate[i].id() == buf.substr(0, tmplate[i].id().size())) {
                logDebug << "FOUND line: " << buf << std::endl;
                rval[inum] = tmplate[i];
                rval[inum].line(lineNum);
                inum++;
            }
    }

    return rval;
}


void MemMeter::initLineInfo(void){
    std::vector<LineInfo> infos;
    infos.push_back(LineInfo("MemTotal", &_total));
    infos.push_back(LineInfo("Buffers", &_fields[1]));
    infos.push_back(LineInfo("Cached", &_fields[2]));
    infos.push_back(LineInfo("SwapCached", &_fields[3]));
    infos.push_back(LineInfo("MemFree", &_fields[4]));

    _MIlineInfos = findLines(infos, MEMFILENAME);
}


void MemMeter::getmemstat(const std::string &fname,
  std::vector<LineInfo> &infos){
    std::ifstream meminfo(fname.c_str());
    if (!meminfo){
        logFatal << "Can not open file : " << fname << std::endl;
    }

    std::string buf;

    // Get the info from the "standard" meminfo file.
    int lineNum = 0;
    size_t fcount = 0; // found count
    while (!meminfo.eof()){
        std::getline(meminfo, buf);
        lineNum++;
        for (size_t i = 0 ; i < infos.size() ; i++)
            if (infos[i].line() == lineNum) {
                std::istringstream line(buf);
                unsigned long val;
                std::string ignore;
                line >> ignore >> val;
                // All stats are in KB.
                // Multiply by 1024 bytes per K
                infos[i].setVal(val*1024.0);
                fcount++;
                break;
            }

        if (fcount >= infos.size())
            break;
    }
}
