//
//  Copyright (c) 1999, 2006, 2015
//  by Thomas Waldmann ( ThomasWaldmann@gmx.de )
//  based on work of Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "raidmeter.h"
#include "xosview.h"
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>


RAIDMeter::RAIDMeter( XOSView *parent, int raiddev)
    : BitFieldMeter( parent, 1, 2, "RAID") {
    _raiddev = raiddev;
    getRAIDstate();
    if(disknum<1)
        disableMeter();
    std::ostringstream os;
    os << "MD" << raiddev << std::ends;
    legend(os.str().c_str());
    if(disknum>=1){
        setfieldlegend("Done/ToDo");
        setNumBits(disknum);
    }
    total_ = 100.0;
}

RAIDMeter::~RAIDMeter( void ){
}

void RAIDMeter::checkevent( void ){

    getRAIDstate();

    for ( int i = 0 ; i < disknum ; i++ ){
        bits_[i] = (working_map[i]=='+');
    }
    fields_[0]=100.0;
    util::fstr(resync_state.substr(resync_state.find('=')+1), fields_[0]);
    fields_[1] = total_ - fields_[1];
    if(fields_[0]<100.0){
        setfieldcolor(0,doneColor_);
        setfieldcolor(1,todoColor_);
    }else{
        setfieldcolor(0,completeColor_);
    }
    setUsed(fields_[0], total_);
    BitFieldMeter::checkevent();
}

void RAIDMeter::checkResources( void ){
    BitFieldMeter::checkResources();
    onColor_ = parent_->allocColor(parent_->getResource("RAIDdiskOnlineColor"));
    offColor_ = parent_->allocColor(
        parent_->getResource("RAIDdiskFailureColor"));
    doneColor_ = parent_->allocColor(
        parent_->getResource("RAIDresyncdoneColor"));
    todoColor_ = parent_->allocColor(
        parent_->getResource("RAIDresynctodoColor"));
    completeColor_= parent_->allocColor(
        parent_->getResource("RAIDresynccompleteColor"));
    priority_  = util::stoi(parent_->getResource("RAIDPriority"));
    setfieldcolor( 0, doneColor_ );
    setfieldcolor( 1, todoColor_ );
    SetUsedFormat(parent_->getResource( "RAIDUsedFormat" ) );
}

// parser for /proc/mdstat

int RAIDMeter::find1(const std::string &key, const std::string &findwhat,
  int num1){
    std::string buf;
    std::ostringstream os;
    os << findwhat << "." << num1 << std::ends;

    return os.str() == key;
}

int RAIDMeter::find2(const std::string &key, const std::string &findwhat,
  int num1, int num2){
    std::string buf;
    std::ostringstream os;
    os << findwhat << "." << num1 << "." << num2 << std::ends;

    return os.str() == key;
}

static const char *RAIDFILE    = "/proc/mdstat";

int RAIDMeter::raidparse(const std::string &cp){
    std::vector<std::string> tokens = util::split(cp, " \n");
    if(tokens.size() == 0)
        return 1;
    bool bval = (tokens.size() >= 2);
    std::string key = tokens[0];
    std::string val;
    if (bval)
        val = tokens[1];

    if(find1(key,"md_state",_raiddev)){
        if(bval)
            state = val;
    }
    else if(find1(key,"md_type",_raiddev)){
        if(bval)
            type = val;
    }
    else if(find1(key,"md_disk_count",_raiddev)){
        if(bval)
            disknum=util::stoi(val);
    }
    else if(find1(key,"md_working_disk_map",_raiddev)){
        if(bval)
            working_map = val;
    }
    else if(find1(key,"md_resync_status",_raiddev)){
        if(bval)
            resync_state = val;
    }
    return 0;
}

void RAIDMeter::getRAIDstate( void ){
    std::ifstream raidfile( RAIDFILE );
    char l[256];

    if ( !raidfile ){
        std::cerr <<"Can not open file : " <<RAIDFILE << std::endl;
        exit( 1 );
    }

    do{
        raidfile.getline(l,256);
    }while((raidparse(l)==0) && (!raidfile.eof()));

//  printf("md0 %s %s %s resync: %s\n",type,state,working_map,resync_state);
}
