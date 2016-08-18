//
//  Copyright (c) 1995, 1996, 1997-2002, 2015, 2016
//  by Brian Grayson (bgrayson@netbsd.org)
//
//  This file was written by Brian Grayson for the NetBSD and xosview
//    projects.
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//

#include "diskmeter.h"
#include "sctl.h"

#if defined(XOSVIEW_FREEBSD)
#include <devstat.h>
#elif defined(XOSVIEW_DFBSD)
#include <sys/devicestat.h>
#endif


DiskMeter::DiskMeter( void )
    : ComDiskMeter(),
      prevreads_(0), prevwrites_(0) {

    getDiskXFerBytes(prevreads_, prevwrites_);
    IntervalTimerStart();
}


std::pair<double, double> DiskMeter::getRate(void) {
    uint64_t reads = 0, writes = 0;
    IntervalTimerStop();
    double t = IntervalTimeInSecs();
    getDiskXFerBytes(reads, writes);
    IntervalTimerStart();

    std::pair<double, double> rval((reads - prevreads_) / t,
      (writes - prevwrites_) / t);
    prevreads_ = reads;
    prevwrites_ = writes;

    return rval;
}


#if defined(XOSVIEW_FREEBSD) || defined(XOSVIEW_DFBSD)
void DiskMeter::getDiskXFerBytes(uint64_t &rbytes,
  uint64_t &wbytes) {

    static SysCtl numdevs_sc("kern.devstat.numdevs");
    static SysCtl devstat_sc("kern.devstat.all");
    static SysCtl version_sc("kern.devstat.version");

    rbytes = wbytes = 0;

    int dsvers = 0;
    if (!version_sc.get(dsvers))
        logFatal << "sysctl(" << version_sc.id() << ") failed." << std::endl;
    if (dsvers != DEVSTAT_VERSION)
        logFatal << "DEVSTAT_VERSION (compiled: " << DEVSTAT_VERSION << ") != "
                 << dsvers << " (in kernel)." << std::endl;

    int ndevs = 0;
    if (!numdevs_sc.get(ndevs))
        logProblem << "sysctl(" << numdevs_sc.id() << ") failed." << std::endl;

    std::vector<char> buf(sizeof(long) + ndevs * sizeof(struct devstat));
    if (!devstat_sc.get(buf))
        logProblem << "sysctl(" << devstat_sc.id() << ") failed." << std::endl;

    // The list of struct devstat is preceeded by a generation number.
    const long *generation = reinterpret_cast<const long *>(buf.data());
    const struct devstat *ds = reinterpret_cast<const struct devstat *>(
        generation + 1);

    for (int i = 0 ; i < ndevs ; i++) {
#if defined(XOSVIEW_FREEBSD)
        rbytes += ds->bytes[DEVSTAT_READ];
        wbytes += ds->bytes[DEVSTAT_WRITE];
#elif defined(XOSVIEW_DFBSD)
        rbytes += ds->bytes_read;
        wbytes += ds->bytes_written;
#endif

        ds++;
    }

    logDebug << "disk read/write: " << (rbytes / 1024) << "k/"
             << (wbytes / 1024) << "k" << std::endl;
}
#endif
