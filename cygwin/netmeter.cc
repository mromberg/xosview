//
//  Copyright (c) 1994, 1995, 2002, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "netmeter.h"
#include "winders.h"

#include <iomanip>
#include <cctype>





NetMeter::NetMeter( XOSView *parent)
    : FieldMeterGraph( parent, 3, "NET", "IN/OUT/IDLE" ), _maxBandwidth(0),
      _inIndex(0) {

    std::vector<std::string> adapters = WinHardware::getAdapters();
    logDebug << "network adapters: " << adapters << std::endl;

    add(adapters,
      PerfQuery::expand("\\Network Interface(*)\\Bytes Received/sec"));

    _inIndex = _query.counters().size();

    add(adapters,
      PerfQuery::expand("\\Network Interface(*)\\Bytes Sent/sec"));


    _query.query();
}


NetMeter::~NetMeter( void ){
}


void NetMeter::checkResources(const ResDB &rdb){
    FieldMeterGraph::checkResources(rdb);

    _maxBandwidth = util::stof(rdb.getResource( "netBandwidth" ));
    setfieldcolor( 0, rdb.getColor( "netInColor" ) );
    setfieldcolor( 1, rdb.getColor( "netOutColor" ) );
    setfieldcolor( 2, rdb.getColor( "netBackground" ) );
    priority_ = util::stoi (rdb.getResource( "netPriority" ));
    useGraph_ = rdb.isResourceTrue( "netGraph" );
    dodecay_ = rdb.isResourceTrue( "netDecay" );
    setUsedFormat (rdb.getResource("netUsedFormat"));
    decayUsed(rdb.isResourceTrue("netUsedDecay"));
}


void NetMeter::checkevent(void) {
    netpair nstats(getStats());

    unsigned long long in = nstats.first;
    unsigned long long out = nstats.second;

//    logDebug << "IN : " << in << std::endl;
//    logDebug << "OUT: " << out << std::endl;


    if ((in + out) > _maxBandwidth) { // display percentages
        total_ = (in + out);
        fields_[0] = in / total_;
        fields_[1] = out / total_;
        fields_[2] = 0;
        total_ = 1.0;
    }
    else {
        total_ = _maxBandwidth;
        fields_[0] = in;
        fields_[1] = out;
        fields_[2] = total_ - fields_[0] - fields_[1];
    }

    setUsed(out + in, _maxBandwidth);
}

NetMeter::netpair NetMeter::getStats(void) {
    _query.query();

    netpair rval(0, 0);
    for (size_t i = 0 ; i < _inIndex ; i++)
        rval.first += _query.counters()[i].longVal();
    for (size_t i = _inIndex ; i < _query.counters().size() ; i++)
        rval.second += _query.counters()[i].longVal();

    return rval;
}


struct AlphaEqual {
    bool operator()(const TCHAR &a, const TCHAR &b) {
        if (!std::isalpha(a) && !std::isalpha(b))
            return true;
        if (a == b)
            return true;
        return false;
    }
};


void NetMeter::add(const std::vector<std::string> &adapters,
  const std::vector<std::string> &plist) {

    for (size_t i = 0 ; i < plist.size() ; i++) {
        std::string iname = PerfQuery::parse(plist[i])["iname"];
        for (size_t j = 0 ; j < adapters.size() ; j++)
            if (std::equal(iname.begin(), iname.end(), adapters[j].begin(),
                AlphaEqual())) {
                if (!_query.add(plist[i])) {
                    logProblem << "failed to add: " << plist[i] << std::endl;
                }
                else {
                    logDebug << "Add: " << plist[i] << std::endl;
                }
                break;
            }
    }
}
