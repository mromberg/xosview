//
//  Copyright (c) 1996, 2004, 2015
//  by Massimiliano Ghilardi ( ghilardi@cibs.sns.it )
//
//  This file may be distributed under terms of the GPL
//

#include "pagemeter.h"
#include "fsutil.h"

#include <fstream>


PageMeter::PageMeter( XOSView *parent, float max )
    : FieldMeterGraph( parent, 3, "PAGE", "IN/OUT/IDLE" ),
      pageinfo_(2, std::vector<float>(2, 0.0)),
      pageindex_(0), maxspeed_(max), _vmstat(false),
      _statFileName("/proc/stat") {

    if (util::fs::isfile("/proc/vmstat")) {
        _vmstat = true;
        _statFileName = "/proc/vmstat";
    }
}


PageMeter::~PageMeter( void ){
}


void PageMeter::checkResources(const ResDB &rdb){
    FieldMeterGraph::checkResources(rdb);

    setfieldcolor( 0, rdb.getColor( "pageInColor" ) );
    setfieldcolor( 1, rdb.getColor( "pageOutColor" ) );
    setfieldcolor( 2, rdb.getColor( "pageIdleColor" ) );
    priority_ = util::stoi (rdb.getResource( "pagePriority" ));
    maxspeed_ *= priority_ / 10.0;
    dodecay_ = rdb.isResourceTrue( "pageDecay" );
    useGraph_ = rdb.isResourceTrue( "pageGraph" );
    setUsedFormat (rdb.getResource("pageUsedFormat"));
    decayUsed(rdb.isResourceTrue("pageUsedDecay"));
}


void PageMeter::checkevent( void ){
    if (_vmstat)
        getvmpageinfo();
    else
        getpageinfo();
}


void PageMeter::updateinfo(void) {
    int oldindex = (pageindex_+1)%2;
    for ( int i = 0; i < 2; i++ ) {
        if ( pageinfo_[oldindex][i] == 0 )
            pageinfo_[oldindex][i] = pageinfo_[pageindex_][i];

        fields_[i] = pageinfo_[pageindex_][i] - pageinfo_[oldindex][i];
        total_ += fields_[i];
    }

    if ( total_ > maxspeed_ )
        fields_[2] = 0.0;
    else {
        fields_[2] = maxspeed_ - total_;
        total_ = maxspeed_;
    }

    setUsed (total_ - fields_[2], maxspeed_);
    pageindex_ = (pageindex_ + 1) % 2;
}


void PageMeter::getvmpageinfo(void) {
    total_ = 0;
    std::string buf;
    std::ifstream stats(_statFileName.c_str());
    if (!stats) {
        logFatal << "Cannot open file : " << _statFileName << std::endl;
    }
    do {
        stats >> buf;
    } while (!stats.eof() && (util::tolower(buf) != "pswpin"));
    stats >>pageinfo_[pageindex_][0];

    do {
        stats >> buf;
    } while (!stats.eof() && (util::tolower(buf) != "pswpout"));
    stats >> pageinfo_[pageindex_][1];

    updateinfo();
}


void PageMeter::getpageinfo( void ){
    total_ = 0;
    std::string buf;
    std::ifstream stats(_statFileName.c_str());

    if ( !stats ){
        logFatal << "Cannot open file : " << _statFileName << std::endl;
    }

    do {
        stats >>buf;
    } while (!stats.eof() && (util::tolower(buf) != "swap"));

    stats >>pageinfo_[pageindex_][0] >>pageinfo_[pageindex_][1];

    updateinfo();
}
