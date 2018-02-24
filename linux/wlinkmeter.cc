//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
#include "wlinkmeter.h"

#include <fstream>


static const char * const WIRELESSFNAME = "/proc/net/wireless";



WLinkMeter::WLinkMeter(void)
    : FieldMeterGraph(2, "WLNK", "LINK/70"),
      _goodColor(0), _poorColor(0), _poorValue(39) {

    // scale is always 0 - 70 (I think)
    _total = 70.0;
}


void WLinkMeter::checkResources(const ResDB &rdb) {

    FieldMeterGraph::checkResources(rdb);

    _goodColor = rdb.getColor("wlinkGoodColor");
    _poorColor = rdb.getColor("wlinkPoorColor");
    _poorValue = std::stoi(rdb.getResource("wlinkPoorValue"));
    setfieldcolor(0, _goodColor);
    setfieldcolor(1, rdb.getColor("wlinkBackground"));
}


void WLinkMeter::checkevent(void) {

    int link = getLink();

    // clamp the range.
    link = std::max(link, 0);
    link = std::min(link, 70);

    setfieldcolor(0, (link < _poorValue) ? _poorColor : _goodColor);
    _total = 70.0;
    _fields[0] = link;
    _fields[1] = _total - link;
    setUsed(link, _total);
}


int WLinkMeter::getLink(void) const {
    std::ifstream ifs(WIRELESSFNAME);
    if (!ifs)
        return -1;  // we won't die in case it is just switched off

    // first two lines are garbage
    ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::string buf;
    ifs >> buf;
    if (!ifs)     // again it may just not be there
        return -1;

    // ok we got a device line.  log any errors beyone this point.
    int rval;
    ifs >> buf >> rval;;
    if (!ifs)
        logFatal << "could not parse: " << WIRELESSFNAME << std::endl;

    return rval;
}
