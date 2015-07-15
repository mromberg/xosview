//
//  Copyright (c) 1999, 2006, 2015
//  by Mike Romberg (mike-romberg@comcast.net)
//
//  This file may be distributed under terms of the GPL
//


#include "diskmeter.h"
#include "xosview.h"
#include "fsutil.h"

#include <fstream>
#include <limits>




static const size_t MAX_PROCSTAT_LENGTH = 2048;


DiskMeter::DiskMeter( XOSView *parent, float max )
    : FieldMeterGraph(parent, 3, "DISK", "READ/WRITE/IDLE"),
      sysfs_read_prev_(0), sysfs_write_prev_(0), _sysfs(false),
      read_prev_(0), write_prev_(0), maxspeed_(max), _vmstat(false),
      _statFileName("/proc/stat") {

    logDebug << "DiskMeter::DiskMeter(): fields_.size(): " << fields_.size()
             << std::endl;

    // first - try sysfs:
    if (util::fs::isdir("/sys/block")) {
        _sysfs = true;
        _statFileName = "/sys/block";
        logDebug << "diskmeter: using sysfs /sys/block" << std::endl;
        getsysfsdiskinfo();

    }
    // try vmstat:
    else if (util::fs::isfile("/proc/vmstat")) {
        _vmstat = true;
        _sysfs  = false;
        _statFileName = "/proc/vmstat";
        getvmdiskinfo();
    }
    else // fall back to stat
        getdiskinfo();
}

DiskMeter::~DiskMeter( void ){
}

void DiskMeter::checkResources( void ) {
    FieldMeterGraph::checkResources();

    setfieldcolor( 0, parent_->getResource("diskReadColor") );
    setfieldcolor( 1, parent_->getResource("diskWriteColor") );
    setfieldcolor( 2, parent_->getResource("diskIdleColor") );
    priority_ = util::stoi (parent_->getResource( "diskPriority" ));
    dodecay_ = parent_->isResourceTrue("diskDecay" );
    useGraph_ = parent_->isResourceTrue( "diskGraph" );
    setUsedFormat(parent_->getResource("diskUsedFormat"));
    decayUsed(parent_->isResourceTrue("diskUsedDecay"));
}

void DiskMeter::checkevent( void ) {
    logDebug << "DiskMeter::checkevent: vmstat=" << _vmstat << ", "
             << "sysfs=" << _sysfs << std::endl;
    if (_vmstat)
        getvmdiskinfo();
    else if ( _sysfs )
        getsysfsdiskinfo();
    else
        getdiskinfo();

    drawfields(parent_->g());

}

// IMHO the logic here is quite broken - but for backward compat UNCHANGED:
void DiskMeter::updateinfo(unsigned long one, unsigned long two,
  int fudgeFactor) {
    // assume each "unit" is 1k.
    // This is true for ext2, but seems to be 512 bytes
    // for vfat and 2k for cdroms
    // work in 512-byte blocks

    // tw: strange, on my system, a ext2fs (read and write)
    // unit seems to be 2048. kernel 2.2.12 and the file system
    // is on a SW-RAID5 device (/dev/md0).

    // So this is a FIXME - but how ???

    float itim = IntervalTimeInMicrosecs();
    unsigned long read_curr = one * fudgeFactor;  // FIXME!
    unsigned long write_curr = two * fudgeFactor; // FIXME!

    // avoid strange values at first call
    if(read_prev_ == 0)
        read_prev_ = read_curr;
    if(write_prev_ == 0)
        write_prev_ = write_curr;

    // calculate rate in bytes per second
    fields_[0] = ((read_curr - read_prev_) * 1e6 * 512) / itim;
    fields_[1] = ((write_curr - write_prev_) * 1e6 * 512) / itim;

    // fix overflow (conversion bug?)
    if (fields_[0] < 0.0)
        fields_[0] = 0.0;
    if (fields_[1] < 0.0)
        fields_[1] = 0.0;

    if (fields_[0] + fields_[1] > total_)
       	total_ = fields_[0] + fields_[1];

    fields_[2] = total_ - (fields_[0] + fields_[1]);

    read_prev_ = read_curr;
    write_prev_ = write_curr;

    setUsed((fields_[0]+fields_[1]), total_);
    IntervalTimerStart();
}

void DiskMeter::getvmdiskinfo(void) {
    logDebug << "getvmdiskinfo()" << std::endl;
    IntervalTimerStop();
    total_ = maxspeed_;
    std::string buf;
    std::ifstream stats(_statFileName);
    unsigned long one, two;

    if ( !stats ) {
        logFatal << "Can not open file : " << _statFileName << std::endl;
    }


    stats >> buf;
    // kernel >= 2.5
    while (!stats.eof() && buf != "pgpgin") {
        stats.ignore(1024, '\n');
        stats >> buf;
    }

    // read first value
    stats >> one;

    while (!stats.eof() && buf != "pgpgout") {
        stats.ignore(1024, '\n');
        stats >> buf;
    }

    // read second value
    stats >> two;

    updateinfo(one, two, 4);
}

void DiskMeter::getdiskinfo( void ) {
    IntervalTimerStop();
    total_ = maxspeed_;
    std::string buf;
    std::ifstream stats(_statFileName);

    if ( !stats ) {
        logFatal << "Can not open file : " << _statFileName << std::endl;
    }

    // Find the line with 'page'
    stats >> buf;
    while (buf.substr(0, 8) != "disk_io:") {
        stats.ignore(MAX_PROCSTAT_LENGTH, '\n');
        stats >> buf;
        if (stats.eof())
            break;
    }

    // read values
    unsigned long one=0, two=0;
    unsigned long read1,write1;
    stats >> buf;
    std::istringstream iss(buf);
    while (iss.good()) {
        // "(%lu,%lu):(%lu,%lu,%lu,%lu,%lu)"
        //  these               ^       ^
        std::streamsize max = std::numeric_limits<std::streamsize>::max();
        iss.ignore(max, ':');
        iss.ignore(max, ',');
        iss.ignore(max, ',');
        iss >> read1;
        iss.ignore(max, ',');
        iss >> write1;
        if (iss.good()) {
            one += read1;
            two += write1;
            stats >> buf;
        }
    }

    updateinfo(one, two, 1);
}

// sysfs version - works with long-long !!
// (no dependency on sector-size here )
void DiskMeter::update_info(unsigned long long rsum, unsigned long long wsum) {
    float itim = IntervalTimeInMicrosecs();

    // avoid strange values at first call
    // (by this - the first value displayed becomes zero)
    if(sysfs_read_prev_ == 0L)
        sysfs_read_prev_  = rsum;
    if(sysfs_write_prev_ == 0L)
        sysfs_write_prev_ = wsum;

    // convert rate from bytes/microsec into bytes/second
    fields_[0] = ((rsum - sysfs_read_prev_ ) * 1e6 ) / itim;
    fields_[1] = ((wsum - sysfs_write_prev_) * 1e6 ) / itim;

    logDebug << "itim: " << itim << std::endl;
    logDebug << "fields: " << fields_[0] << ", " << fields_[1] << std::endl;

    // fix overflow (conversion bug?)
    if (fields_[0] < 0.0)
        fields_[0] = 0.0;
    if (fields_[1] < 0.0)
        fields_[1] = 0.0;

    // bump up max total:
    if (fields_[0] + fields_[1] > total_)
        total_ = fields_[0] + fields_[1];

    fields_[2] = total_ - (fields_[0] + fields_[1]);

    // save old vals for next round
    sysfs_read_prev_  = rsum;
    sysfs_write_prev_ = wsum;

    logDebug << "setUsed(): "
             << fields_[0]/total_ << ", "
             << fields_[1]/total_ << ", "
             << fields_[2]/total_ << ", "
             << total_/total_ << std::endl;
    setUsed((fields_[0]+fields_[1]), total_);

    IntervalTimerStart();
}



// XXX: sysfs - read Documentation/iostats.txt !!!
// extract stats from /sys/block/*/stat
// each disk reports a 32bit u_int, which can WRAP around
// XXX: currently sector-size is fixed 512bytes
//      (would need a sysfs-val for sect-size)

void DiskMeter::getsysfsdiskinfo( void ) {
    logDebug << "DiskMeter::getsysfsdiskinfo()" << std::endl;
    // field-3: sects read since boot (but can wrap!)
    // field-7: sects written since boot (but can wrap!)
    // just sum up everything in /sys/block/*/stat

    std::string sysfs_dir = _statFileName;
    std::string disk;
    std::ifstream diskstat;

    // the sum of all disks:
    unsigned long long all_bytes_read,all_bytes_written;

    // ... while this is only one disk's value:
    unsigned long sec_read,sec_written;
    unsigned long sect_size;

    unsigned long dummy;

    IntervalTimerStop();
    total_ = maxspeed_;

    if (!util::fs::isdir(_statFileName)) {
        logDebug << "sysfs: Cannot find directory : "
                 << _statFileName << std::endl;
        return;
    }

    // reset all sums
    all_bytes_read=all_bytes_written=0L;
    sect_size=0L;

    // visit every /sys/block/*/stat and sum up the values:
    std::vector<std::string> dir = util::fs::listdir(_statFileName);
    for (size_t di = 0 ; di < dir.size(); di++) {
        logDebug << "dirent->d_name: " << dir[di] << std::endl;
        std::string dname(dir[di]);
        if (dname == "." || dname == "..")
            continue;

        disk = sysfs_dir + "/" + dname;
        logDebug << "stat(" << disk << ")" << std::endl;

        if (util::fs::isdir(disk)) {
            // is a dir, locate 'stat' file in it
            disk += "/stat";
            if (util::fs::isfile(disk)) {
                logDebug << "disk stat: " << disk << std::endl;
                diskstat.open(disk.c_str());
                if ( diskstat.good() ) {
                    sec_read=sec_written=0L;
                    diskstat >> dummy >> dummy >> sec_read >> dummy
                             >> dummy >> dummy >> sec_written;
                    logDebug << "read stats from " << disk
                             << sec_read << sec_written << std::endl;

                    sect_size = 512; // XXX: not always true

                    // XXX: ignoring wrap around case for each disk
                    // (would require saving old vals for each disk etc..)
                    all_bytes_read    += (unsigned long long) sec_read
                        * (unsigned long long) sect_size;
                    all_bytes_written += (unsigned long long) sec_written
                        * (unsigned long long) sect_size;

                    logDebug << "disk stat: " << disk << " | "
                             << "read: " << sec_read << ", "
                             << "written: " << sec_written << std::endl;
                    diskstat.close();
                    diskstat.clear();
                }
                else {
                    logDebug << "disk stat open: " << disk << std::endl;
                }
            }
            else {
                logDebug << "disk stat is not file: " << disk << std::endl;
            }
        }
        else {
            logDebug << "disk is not dir: " << disk << std::endl;
        }
    } // for
    logDebug << "disk: read: " << all_bytes_read << ", "
             << "written: " << all_bytes_written << std::endl;
    update_info(all_bytes_read, all_bytes_written);
}
