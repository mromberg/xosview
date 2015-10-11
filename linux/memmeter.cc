//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "memmeter.h"

#include <fstream>
#include <sstream>
#include <iomanip>


static const char * const MEMFILENAME = "/proc/meminfo";



MemMeter::MemMeter( XOSView *parent ) : FieldMeterGraph( parent, 5, "MEM",
  "USED/BUFF/CACHE/SCACHE/FREE" ){
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
             << "t " << total_ * TOMEG << " "
             << "used "    << fields_[0] * TOMEG << " "
             << "buf "     << fields_[1] * TOMEG << " "
             << "cache "   << fields_[2] * TOMEG << " "
             << "swcache " << fields_[3] * TOMEG << " "
             << "free "    << fields_[4] * TOMEG
             << std::endl;
}

// FIXME: /proc/memstat and /proc/meminfo don't seem to correspond
// maybe it is time to fix this in the kernel and get real infos ...

void MemMeter::getmeminfo( void ){
    getmemstat(MEMFILENAME, _MIlineInfos);

    fields_[0] = total_ - fields_[4] - fields_[3] - fields_[2] - fields_[1];

    if (total_)
        FieldMeterDecay::setUsed (total_ - fields_[3] - fields_[4], total_);
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
    infos.push_back(LineInfo("MemTotal", &total_));
    infos.push_back(LineInfo("Buffers", &fields_[1]));
    infos.push_back(LineInfo("Cached", &fields_[2]));
    infos.push_back(LineInfo("SwapCached", &fields_[3]));
    infos.push_back(LineInfo("MemFree", &fields_[4]));

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
