//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
#include "tzonemeter.h"
#include "fsutil.h"

static const char * const TZDIR  = "/sys/class/thermal";
static const char * const TZSDIR = "thermal_zone";


TZoneMeter::TZoneMeter(XOSView *parent, size_t zoneNum)
    : FieldMeterGraph(parent, 2, "TZ" + util::repr(zoneNum),
      "DEG C/USED", true, true, true), _peak(100.0),
      _hotTrip(70), _critTrip(84),
      _normColor(0), _hotColor(0), _critColor(0) {

    _tempFName = std::string(TZDIR) + "/" + TZSDIR + util::repr(zoneNum)
        + "/" + "temp";
}

TZoneMeter::~TZoneMeter(void) {
}

void TZoneMeter::checkResources( void ) {

    FieldMeterGraph::checkResources();

    // change "tzone" to real meter name
    _normColor = parent_->g().allocColor(parent_->getResource(
          "tzoneForeGround"));
    _hotColor = parent_->g().allocColor(parent_->getResource(
          "tzoneHotColor"));
    _critColor = parent_->g().allocColor(parent_->getResource(
          "tzoneCritColor"));
    setfieldcolor(0, _normColor);
    setfieldcolor(1, parent_->getResource("tzoneBackground"));
    _peak = util::stof(parent_->getResource("tzonePeak"));
    _hotTrip = util::stof(parent_->getResource("tzoneHotTrip"));
    _critTrip = util::stof(parent_->getResource("tzoneCritTrip"));
    priority_ = util::stoi(parent_->getResource("tzonePriority"));
    dodecay_ = parent_->isResourceTrue("tzoneDecay");
    useGraph_ = parent_->isResourceTrue("tzoneGraph");
    setUsedFormat(parent_->getResource("tzoneUsedFormat"));
    decayUsed(parent_->isResourceTrue("tzoneUsedDecay"));
}

void TZoneMeter::checkevent( void ) {

    // Read the temperature.  The docs say.
    // Unit: millidegree Celsius
    unsigned long long temp = 0;
    if (!util::fs::readFirst(_tempFName, temp))
        logFatal << "error reading: " << _tempFName << std::endl;
    float ctemp = static_cast<float>(temp) / 1000.0;

    // set the fields
    total_ = _peak;
    fields_[0] = ctemp;
    fields_[1] = total_ - fields_[0];

    // and the colors
    unsigned long ocolor = fieldcolor(0);
    if (ctemp >= _critTrip)
        setfieldcolor(0, _critColor);
    else if (ctemp >= _hotTrip)
        setfieldcolor(0, _hotColor);
    else
        setfieldcolor(0, _normColor);

    if (ocolor != fieldcolor(0))
        drawLegend(parent_->g());
    setUsed(ctemp, 1.0);
    drawfields(parent_->g());
}

size_t TZoneMeter::count(void) {
    if (util::fs::isdir(TZDIR)) {
        std::vector<std::string> flist = util::fs::listdir(TZDIR);
        size_t rval = 0;
        std::string tzsdir(TZSDIR);
        for (size_t i = 0 ; i < flist.size() ; i++)
            if (flist[i].substr(0, tzsdir.size()) == tzsdir)
                rval++;

        return rval;
    }
    else
        logProblem << "directory does not exist: " << TZDIR << std::endl;
    return 0;
}
