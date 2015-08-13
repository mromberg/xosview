//
//  Copyright (c) 1994, 1995, 2015 by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  NetBSD port:
//  Copyright (c) 1995, 1996, 1997-2002 by Brian Grayson (bgrayson@netbsd.org)
//
//  This file was written by Brian Grayson for the NetBSD and xosview
//    projects.
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//

#include "cpumeter.h"
#include "kernel.h"
#include "defines.h"

// for CPUSTATES
#if defined(XOSVIEW_NETBSD)
#include <sys/sched.h>
#elif defined(XOSVIEW_OPENBSD)
#if defined(HAVE_SYS_DKSTAT_H)
#include <sys/dkstat.h>
#elif defined(HAVE_SYS_SCHED_H)
#include <sys/sched.h>
#endif
#else
#include <sys/resource.h>
#endif




CPUMeter::CPUMeter( XOSView *parent, unsigned int nbr )
    : FieldMeterGraph( parent, 5, "CPU", "USR/NICE/SYS/INT/FREE" ),
      cputime_(2, std::vector<uint64_t>(CPUSTATES, 0)),
      cpuindex_(0), nbr_(nbr) {

    BSDCPUInit();

    title(std::string("CPU") + util::repr(nbr_ - 1));
}


CPUMeter::~CPUMeter( void ) {
}


void CPUMeter::checkResources(const ResDB &rdb) {
    FieldMeterGraph::checkResources(rdb);

    setfieldcolor( 0, rdb.getColor("cpuUserColor") );
    setfieldcolor( 1, rdb.getColor("cpuNiceColor") );
    setfieldcolor( 2, rdb.getColor("cpuSystemColor") );
    setfieldcolor( 3, rdb.getColor("cpuInterruptColor") );
    setfieldcolor( 4, rdb.getColor("cpuFreeColor") );
    priority_ = util::stoi( rdb.getResource("cpuPriority") );
    dodecay_ = rdb.isResourceTrue("cpuDecay");
    useGraph_ = rdb.isResourceTrue("cpuGraph");
    setUsedFormat( rdb.getResource("cpuUsedFormat") );
}


void CPUMeter::checkevent( void ) {
    getcputime();
    drawfields(parent_->g());
}


void CPUMeter::getcputime( void ) {
    uint64_t tempCPU[CPUSTATES];
    total_ = 0;

    BSDGetCPUTimes(tempCPU, nbr_);

    int oldindex = (cpuindex_ + 1) % 2;
    for (int i = 0; i < CPUSTATES; i++) {
        cputime_[cpuindex_][i] = tempCPU[i];
        fields_[i] = cputime_[cpuindex_][i] - cputime_[oldindex][i];
        total_ += fields_[i];
    }
    if (total_) {
        setUsed(total_ - fields_[4], total_);
        cpuindex_ = (cpuindex_ + 1) % 2;
    }
}
