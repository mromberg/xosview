//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "memmeter.h"
#include "xosview.h"
#include <fstream>
#include <sstream>
#include <stdlib.h>

static const char MEMFILENAME[] = "/proc/meminfo";
static const char MEMSTATFNAME[] = "/proc/memstat";

MemMeter::MemMeter( XOSView *parent ) : FieldMeterGraph( parent, 4, "MEM",
  "USED+SHAR/BUFF/CACHE/FREE" ){
    _shAdj = -1;

    // Check and see if the memstat module has been loaded
    std::ifstream test(MEMSTATFNAME);
    if (test){
        setNumFields(5);
        legend("USED/SHAR/BUFF/CACHE/FREE");
        _shAdj = 0;
    }

    initLineInfo();
}

MemMeter::~MemMeter( void ){
}

void MemMeter::checkResources( void ){
    FieldMeterGraph::checkResources();

    setfieldcolor( 0, parent_->getResource( "memUsedColor" ) );
    if (_shAdj == 0)
        setfieldcolor( 1, parent_->getResource( "memSharedColor" ) );
    setfieldcolor( 2 + _shAdj, parent_->getResource( "memBufferColor" ) );
    setfieldcolor( 3 + _shAdj, parent_->getResource( "memCacheColor" ) );
    setfieldcolor( 4 + _shAdj, parent_->getResource( "memFreeColor" ) );
    priority_ = util::stoi (parent_->getResource( "memPriority" ));
    dodecay_ = parent_->isResourceTrue( "memDecay" );
    useGraph_ = parent_->isResourceTrue( "memGraph" );
    setUsedFormat (parent_->getResource("memUsedFormat"));
}

void MemMeter::checkevent( void ){
    getmeminfo();
    /* for debugging (see below)
       printf("t %4.1f used %4.1f shr %4.1f buf %4.1f cache %4.1f free %4.1f\n",
       total_/1024.0/1024.0,
       fields_[0]/1024.0/1024.0, fields_[1]/1024.0/1024.0,
       fields_[2]/1024.0/1024.0, fields_[3]/1024.0/1024.0,
       fields_[4]/1024.0/1024.0);
    */
    drawfields();
}

// FIXME: /proc/memstat and /proc/meminfo don't seem to correspond
// maybe it is time to fix this in the kernel and get real infos ...

void MemMeter::getmeminfo( void ){
    getmemstat(MEMFILENAME, _MIlineInfos);
    if (_shAdj == 0)
        getmemstat(MEMSTATFNAME, _MSlineInfos);

    if (_shAdj == 0){
        fields_[3] -= fields_[1]; // cache size seems to contain shared size !?
                                  // without this "used" sometimes gets < 0 !
        fields_[0] = total_ - fields_[4] - fields_[3] - fields_[2] - fields_[1];
    }else{
        fields_[0] = total_ - fields_[3] - fields_[2] - fields_[1];
    }

    if (total_)
        FieldMeterDecay::setUsed (total_ - fields_[4 + _shAdj], total_);
}

std::vector<MemMeter::LineInfo> MemMeter::findLines(
    const std::vector<LineInfo> &tmplate, const std::string &fname){
    std::ifstream meminfo(fname.c_str());
    if (!meminfo){
        std::cerr << "Can not open file : " << fname << std::endl;
        exit(1);
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
    infos.push_back(LineInfo("MemFree", &fields_[4 + _shAdj]));
    infos.push_back(LineInfo("Buffers", &fields_[2 + _shAdj]));
    infos.push_back(LineInfo("Cached", &fields_[3 + _shAdj]));
    _MIlineInfos = findLines(infos, MEMFILENAME);

    std::vector<LineInfo> msinfos;
    msinfos.push_back(LineInfo("Shared", &fields_[1]));

    if (_shAdj == 0)
        _MSlineInfos = findLines(msinfos, MEMSTATFNAME);
}

void MemMeter::getmemstat(const std::string &fname,
  std::vector<LineInfo> &infos){
    std::ifstream meminfo(fname.c_str());
    if (!meminfo){
        std::cerr << "Can not open file : " << fname << std::endl;
        exit(1);
    }

    char buf[256];

    // Get the info from the "standard" meminfo file.
    int lineNum = 0;
    size_t inum = 0;
    while (!meminfo.eof()){
        meminfo.getline(buf, 256);
        lineNum++;
        if (lineNum != infos[inum].line())
            continue;

        std::string sline(buf, 256);
        std::istringstream line(sline);
        unsigned long val;
        std::string ignore;
        line >> ignore >> val;
        /*  All stats are in KB.  */
        infos[inum].setVal(val*1024.0);	/*  Multiply by 1024 bytes per K  */

        inum++;
        if (inum >= infos.size())
            break;
    }
}
