//
//  Copyright (c) 2015
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//
#include "loadmeter.h"

#include <kstat.h>


LoadMeter::LoadMeter(XOSView *parent, kstat_ctl_t *_kc)
    : FieldMeterGraph(parent, 2, "LOAD", "PROCS/MIN", 1, 1, 0),
      procloadcol_(0), warnloadcol_(0), alarmThreshold(0),
      kc(_kc), ksp(0) {

    std::string ustr("unix"), smstr("system_misc");
    ksp = kstat_lookup(kc, const_cast<char *>(ustr.c_str()), 0,
      const_cast<char *>(smstr.c_str()));
    if (ksp == NULL)
        logFatal << "kstat_lookup() failed." << std::endl;
}

LoadMeter::~LoadMeter(void){
}

void LoadMeter::checkResources(void) {

    FieldMeterGraph::checkResources();

    warnloadcol_ = parent_->g().allocColor(
        parent_->getResource("loadWarnColor" ));
    procloadcol_ = parent_->g().allocColor(
        parent_->getResource("loadProcColor"));

    setfieldcolor(0, procloadcol_);
    setfieldcolor(1, parent_->getResource("loadIdleColor"));
    priority_ = util::stoi (parent_->getResource("loadPriority"));
    dodecay_ = parent_->isResourceTrue("loadDecay");
    useGraph_ = parent_->isResourceTrue("loadGraph");
    setUsedFormat(parent_->getResource("loadUsedFormat"));

    alarmThreshold = util::stoi (parent_->getResource("loadWarnThreshold"));

    if (dodecay_){
        /*
         * Warning: Since the loadmeter changes scale
         * occasionally, old decay values need to be rescaled.
         * However, if they are rescaled, they could go off the
         * edge of the screen.  Thus, for now, to prevent this
         * whole problem, the load meter can not be a decay
         * meter.  The load is a decaying average kind of thing
         * anyway, so having a decaying load average is
         * redundant.
         */
        logProblem << "The loadmeter can not be configured as a decay\n"
                   << "  meter.  See the source code (" << __FILE__
                   << ") for further\n"
                   << "  details.\n";
        dodecay_ = 0;
    }
}

void LoadMeter::checkevent(void) {
    getloadinfo();
    drawfields(parent_->g());
}

void LoadMeter::getloadinfo(void) {
#if 1
    kstat_named_t *k;

    if (kstat_read(kc, ksp, NULL) == -1)
        logFatal << "kstat_read() failed" << std::endl;

    std::string avOneMin("avenrun_1min");
    k = (kstat_named_t *)kstat_data_lookup(ksp,
      const_cast<char *>(avOneMin.c_str()));
    if (k == NULL)
        logFatal << "kstat_data_lookup() failed." << std::endl;

    fields_[0] = (double)k->value.l / (1l << 8);
#else
    double oneMinLoad;

    getloadavg(&oneMinLoad, 1);
    fields_[0] = oneMinLoad;
#endif

    if (fields_[0] > alarmThreshold) {
        if (total_ == alarmThreshold) {
            setfieldcolor(0, warnloadcol_);
            if (dolegends())
                drawLegend(parent_->g());
        }
        total_ = fields_[1] = 20;
    }
    else {
        if (total_ == 20) {
            setfieldcolor(0, procloadcol_);
            if (dolegends())
                drawLegend(parent_->g());
        }
        total_ = fields_[1] = alarmThreshold;
    }
    setUsed(fields_[0], total_);
}
