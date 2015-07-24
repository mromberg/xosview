//
//  Copyright (c) 2015
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
//  This file may be distributed under terms of the GPL
//

#include "loadmeter.h"



#ifdef HAVE_GETLOADAVG
#include <sys/loadavg.h>
#else
#ifndef FSCALE
#define FSCALE (1<<8)
#endif
#endif



LoadMeter::LoadMeter(XOSView *parent, kstat_ctl_t *_kc)
    : FieldMeterGraph(parent, 2, "LOAD", "PROCS/MIN", 1, 1, 0),
      procloadcol(0), warnloadcol(0), critloadcol(0),
      warnThreshold(0), critThreshold(0),
      old_cpu_speed(0), cur_cpu_speed(0),
      lastalarmstate(-1), do_cpu_speed(false),
      cpulist(KStatList::getList(_kc, KStatList::CPU_INFO)),
      kc(_kc), ksp(0) {

    total_ = -1;

#ifndef HAVE_GETLOADAVG
    ksp = kstat_lookup(kc, const_cast<char *>("unix"), 0,
      const_cast<char *>("system_misc"));
    if (ksp == NULL)
        logFatal << "kstat_lookup() failed." << std::endl;
#endif
}


LoadMeter::~LoadMeter(void) {
}


void LoadMeter::checkResources(void) {

    FieldMeterGraph::checkResources();

    warnloadcol = parent_->g().allocColor(parent_->getResource(
          "loadWarnColor"));
    procloadcol = parent_->g().allocColor(parent_->getResource(
          "loadProcColor"));
    critloadcol = parent_->g().allocColor(parent_->getResource(
          "loadCritColor"));

    setfieldcolor(0, procloadcol);
    setfieldcolor(1, parent_->getResource("loadIdleColor"));
    priority_ = util::stoi (parent_->getResource("loadPriority"));
    dodecay_ = parent_->isResourceTrue("loadDecay");
    useGraph_ = parent_->isResourceTrue("loadGraph");
    setUsedFormat(parent_->getResource("loadUsedFormat"));
    do_cpu_speed = parent_->isResourceTrue("loadCpuSpeed");

    std::string warn = parent_->getResource("loadWarnThreshold");
    if (warn == "auto")
        warnThreshold = sysconf(_SC_NPROCESSORS_ONLN);
    else
        warnThreshold = util::stoi(warn);

    std::string crit = parent_->getResource("loadCritThreshold");
    if (crit == "auto")
        critThreshold = warnThreshold * 4;
    else
        critThreshold = util::stoi(crit);

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
                   << "meter.  See the source code ("
                   << __FILE__ << ") for further\n"
                   << "  details.\n";
        dodecay_ = 0;
    }
}


void LoadMeter::checkevent(void) {
    getloadinfo();
    if (do_cpu_speed) {
        getspeedinfo();
        if (old_cpu_speed != cur_cpu_speed) {
            // update the legend:
            char l[32];
            snprintf(l, 32, "PROCS/MIN %d MHz", cur_cpu_speed);
            legend(l);
            drawLegend(parent_->g());
        }
    }
    drawfields(parent_->g());
}


void LoadMeter::getloadinfo(void) {
    int alarmstate;

#ifndef HAVE_GETLOADAVG
    // This code is mainly for Solaris 6 and earlier, but should work on
    // any version.
    kstat_named_t *k;

    if (kstat_read(kc, ksp, NULL) == -1)
        logFatal << "kstat_read() failed." << std::endl;

    k = (kstat_named_t *)kstat_data_lookup(ksp,
      const_cast<char *>("avenrun_1min"));
    if (k == NULL)
        logFatal << "kstat_data_lookup() failed." << std::endl;

    fields_[0] = kstat_to_double(k) / FSCALE;
#else
    // getloadavg() if found on Solaris 7 and newer.
    double val;
    getloadavg(&val, 1);
    fields_[0] = val;
#endif

    if (fields_[0] <  warnThreshold)
        alarmstate = 0;
    else if (fields_[0] >= critThreshold)
        alarmstate = 2;
    else /* if fields_[0] >= warnThreshold */
        alarmstate = 1;

    if (alarmstate != lastalarmstate) {
        if (alarmstate == 0)
            setfieldcolor(0, procloadcol);
        else if (alarmstate == 1)
            setfieldcolor(0, warnloadcol);
        else /* if alarmstate == 2 */
            setfieldcolor(0, critloadcol);
        drawLegend(parent_->g());
        lastalarmstate = alarmstate;
    }

    // Adjust total to next power-of-two of the current load.
    if ( (fields_[0]*5.0 < total_ && total_ > 1.0) || fields_[0] > total_ ) {
        unsigned int i = fields_[0];
        // i = 2^n - 1
        i |= i >> 1;
        i |= i >> 2;
        i |= i >> 4;
        i |= i >> 8;
        i |= i >> 16;

        total_ = i + 1;
    }

    fields_[1] = total_ - fields_[0];
    setUsed(fields_[0], total_);
}


void LoadMeter::getspeedinfo(void) {
    unsigned int total_mhz = 0, i = 0;
    kstat_named_t *k;
    kstat_t *cpu;
    cpulist->update(kc);

    for (i = 0; i < cpulist->count(); i++) {
        cpu = (*cpulist)[i];
        if (kstat_read(kc, cpu, NULL) == -1)
            logFatal << "kstat_read() failed." << std::endl;

        // Try current_clock_Hz first (needs frequency scaling support),
        // then clock_MHz.
        k = (kstat_named_t *)kstat_data_lookup(cpu,
          const_cast<char *>("current_clock_Hz"));
        if (k == NULL) {
            k = (kstat_named_t *)kstat_data_lookup(cpu,
              const_cast<char *>("clock_MHz"));
            if (k == NULL)
                logFatal << "CPU speed is not available." << std::endl;

            logDebug << "Speed of cpu " << i << " is "
                     << kstat_to_ui64(k) << std::endl;

            total_mhz += kstat_to_ui64(k);
        }
        else {
            logDebug << "Speed of cpu " << i << " is "
                     << kstat_to_ui64(k) << std::endl;

            total_mhz += ( kstat_to_ui64(k) / 1000000 );
        }
    }
    old_cpu_speed = cur_cpu_speed;
    cur_cpu_speed = ( i > 0 ? total_mhz / i : 0 );
}
