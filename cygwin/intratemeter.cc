//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "intratemeter.h"

#include <fstream>
#include <limits>

static const char * const STATFNAME = "/proc/stat";

IrqRateMeter::IrqRateMeter( XOSView *parent )
    : FieldMeterGraph( parent, 2, "IRQR", "IRQs per sec/IDLE", 1, 1, 0 ),
      _peak(100), _last(getIntCount()), _timer(true) {
}


IrqRateMeter::~IrqRateMeter( void ) {
}


void IrqRateMeter::checkResources(const ResDB &rdb) {
    FieldMeterGraph::checkResources(rdb);
    setfieldcolor(0, rdb.getColor("irqrateUsedColor"));
    setfieldcolor(1, rdb.getColor("irqrateIdleColor"));

    priority_ = util::stoi(rdb.getResource("irqratePriority"));
    dodecay_ = rdb.isResourceTrue("irqrateDecay");
    useGraph_ = rdb.isResourceTrue("irqrateGraph");
    setUsedFormat(rdb.getResource("irqrateUsedFormat"));
    decayUsed(rdb.isResourceTrue("irqrateUsedDecay"));
    _peak = util::stoi(rdb.getResource("irqratePeak"));
    total_ = _peak;
}


void IrqRateMeter::checkevent( void ){
    unsigned long long curr = getIntCount();
    _timer.stop();
    double interval(_timer.report_usecs());
    _timer.start();

    float rate = static_cast<float>(curr - _last) / interval * 10000;
    _last = curr;

    setUsed(rate, 1.0);

    if (rate > _peak)
        rate = _peak;

    fields_[0] = rate;
    fields_[1] = total_ - rate;
}

unsigned long long IrqRateMeter::getIntCount(void) const {

    std::ifstream ifs(STATFNAME);
    if (!ifs)
        logFatal << "Could not open: " << STATFNAME << std::endl;

    // Read until we are on the intr line
    while (!ifs.eof()) {
        std::string name;
        ifs >> name;
        if (!ifs.eof() && !ifs.fail()) {
            if (name == "intr") {
                unsigned long long count;
                ifs >> count;
                if (ifs.fail())
                    logFatal << "failed parsing: " << STATFNAME
                             << std::endl;
                return count;
            }
            ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }

    logProblem << "failed to find intr line in: " << STATFNAME << std::endl;
    return 0;
}
