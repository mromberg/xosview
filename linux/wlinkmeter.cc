//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
#include "wlinkmeter.h"

#include <fstream>
#include <limits>

static const char * const WIRELESSFNAME = "/proc/net/wireless";

WLinkMeter::WLinkMeter(XOSView *parent)
    : FieldMeterGraph(parent, 2, "WLNK", "LINK/70", true, true, true),
      _goodColor(0), _poorColor(0), _poorValue(39) {

    // scale is always 0 - 70 (I think)
    total_ = 70.0;
}

WLinkMeter::~WLinkMeter(void) {
}

void WLinkMeter::checkResources( void ) {

    FieldMeterGraph::checkResources();

    _goodColor = parent_->g().allocColor(parent_->getResource(
          "wlinkGoodColor"));
    _poorColor = parent_->g().allocColor(parent_->getResource(
          "wlinkPoorColor"));
    _poorValue = util::stoi(parent_->getResource(
          "wlinkPoorValue"));
    setfieldcolor(0, _goodColor);
    setfieldcolor(1, parent_->getResource("wlinkBackground"));

    priority_ = util::stoi(parent_->getResource("wlinkPriority"));
    dodecay_ = parent_->isResourceTrue("wlinkDecay");
    useGraph_ = parent_->isResourceTrue("wlinkGraph");
    setUsedFormat(parent_->getResource("wlinkUsedFormat"));
    decayUsed(parent_->isResourceTrue("wlinkUsedDecay"));
}

void WLinkMeter::checkevent( void ) {

    int link = getLink();

    if (link < 0)
        link = 0;
    if (link > 70.0)
        link = 70;

    unsigned long ocolor = fieldcolor(0);
    if (link < _poorValue)
        setfieldcolor(0, _poorColor);
    else
        setfieldcolor(0, _goodColor);
    if (ocolor != fieldcolor(0))
        drawLabels(parent_->g());

    total_ = 70.0;
    fields_[0] = link;
    fields_[1] = total_ - link;
    setUsed(link, total_);
    drawfields(parent_->g());
}

int WLinkMeter::getLink(void) const {
    std::ifstream ifs(WIRELESSFNAME);
    if (!ifs)
        return -1;  // we won't die in case it is just switched off

    // first two lines are garbage
    ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::string buf;
    int rval;
    ifs >> buf;

    if (!ifs)     // again it may just not be there
        return -1;

    // ok we got a device line.  log any errors beyone this point.
    ifs >> buf >> rval;;

    if (!ifs)
        logFatal << "could not parse: " << WIRELESSFNAME << std::endl;

    return rval;
}
