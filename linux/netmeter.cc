//
//  Copyright (c) 1994, 1995, 2002, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "netmeter.h"

#include <fstream>
#include <iostream>
#include <cerrno>
#include <iomanip>
#include <string>
#include <limits>


static const char *NETFILENAME = "/proc/net/dev";


NetMeter::NetMeter( XOSView *parent)
    : FieldMeterGraph( parent, 3, "NET", "IN/OUT/IDLE" ), _maxBandwidth(0),
      _netfilename(NETFILENAME) {
    _last = getStats();
    _timer.start();
}

NetMeter::~NetMeter( void ){
}

void NetMeter::checkResources( void ){
    FieldMeterGraph::checkResources();

    _maxBandwidth = util::stof(parent_->getResource( "netBandwidth" ));
    setfieldcolor( 0, parent_->getResource( "netInColor" ) );
    setfieldcolor( 1, parent_->getResource( "netOutColor" ) );
    setfieldcolor( 2, parent_->getResource( "netBackground" ) );
    priority_ = util::stoi (parent_->getResource( "netPriority" ));
    useGraph_ = parent_->isResourceTrue( "netGraph" );
    dodecay_ = parent_->isResourceTrue( "netDecay" );
    setUsedFormat (parent_->getResource("netUsedFormat"));
}

void NetMeter::checkevent(void) {
    netpair nstats(getStats());
    _timer.stop();
    double etime = _timer.report_usecs() / 1000000; // now in seconds
    _timer.start();

    unsigned long long in = nstats.first - _last.first;
    unsigned long long out = nstats.second - _last.second;

    //logDebug << "IN : " << nstats.first << " : " << _last.first << std::endl;
    //logDebug << "OUT: " << nstats.second << " : " << _last.second << std::endl;

    _last = nstats;

    logDebug << "read: " << in << ", " << out
             << " in: " << etime << " seconds" << std::endl;
    logDebug << "rate: " << std::setprecision(1)
             << (in / etime) / 1000
             << "/" << (out / etime) / 1000 << std::endl;

    if (((in + out) / etime) > _maxBandwidth) { // display percentages
        total_ = (in + out) / etime;
        fields_[0] = (in / etime) / total_;
        fields_[1] = (out / etime) / total_;
        fields_[2] = 0;
        total_ = 1.0;
    }
    else {
        total_ = _maxBandwidth;
        fields_[0] = (in / etime);
        fields_[1] = (out / etime);
        fields_[2] = total_ - fields_[0] - fields_[1];
    }

    setUsed((out + in) / etime, _maxBandwidth);
    drawfields(parent_->g());
}

NetMeter::netpair NetMeter::getStats(void) {
    // Returns total bytes in/out
    std::ifstream ifs(_netfilename.c_str());
    if (!ifs)
        logFatal << "can not open: " << _netfilename << std::endl;

    // toss first two lines (captions)
    ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if (!ifs)
        logFatal << "error reading: " << _netfilename << std::endl;

    // each remaining line is stats for an interface.
    unsigned long long read=0, write=0; // totals
    while(!ifs.eof()) {
        std::string buf;
        // The caption says these are in bytes
        unsigned long long receive=0, trans=0;
        ifs >> buf;
        if (buf == "lo:") { // skip the loopback (it is crazy fast
            ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        if (ifs.eof()) // yea we will really hit EOF here
            break;
        ifs >> receive;

        // The transmit is the eighth one down the row
        for (size_t i = 0 ; i < 7 ; i++)
            ifs >> trans;
        ifs >> trans;

        // don't need the rest of the line
        ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (!ifs)
            logFatal << "error reading2: " << _netfilename << std::endl;

        read += receive;
        write += trans;
    }

    return netpair(read, write);
}
