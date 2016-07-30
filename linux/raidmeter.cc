//
//  Copyright (c) 1999, 2006, 2015, 2016
//  by Thomas Waldmann ( ThomasWaldmann@gmx.de )
//  based on work of Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "raidmeter.h"


RAIDMeter::RAIDMeter( int raiddev)
    : BitFieldMeter( 1, 2, "RAID"),
      _raiddev(raiddev),
      _doneColor(0), _todoColor(0) {

    legend("MD" + util::repr(raiddev));
    setNumBits(3);
    total_ = 100.0;
}


RAIDMeter::~RAIDMeter( void ){
}


void RAIDMeter::checkevent( void ){
}


void RAIDMeter::checkResources(const ResDB &rdb){
    BitFieldMeter::checkResources(rdb);

    _doneColor = rdb.getColor("RAIDresyncdoneColor");
    _todoColor = rdb.getColor("RAIDresynctodoColor");

    setfieldcolor( 0, _doneColor );
    setfieldcolor( 1, _todoColor );
}
