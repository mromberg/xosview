//
//  Copyright (c) 1999, 2006, 2015
//  by Mike Romberg (mike-romberg@comcast.net)
//
//  This file may be distributed under terms of the GPL
//

#ifndef DISKMETER_H
#define DISKMETER_H

#include "fieldmetergraph.h"



class DiskMeter : public FieldMeterGraph {
public:
    DiskMeter( XOSView *parent, float max );
    ~DiskMeter( void );

    std::string name( void ) const { return "DiskMeter"; }
    void checkevent( void );

    void checkResources(const ResDB &rdb);

private:
    // sysfs:
    unsigned long long sysfs_read_prev_;
    unsigned long long sysfs_write_prev_;
    bool _sysfs;
    unsigned long int read_prev_;
    unsigned long int write_prev_;
    float maxspeed_;
    bool _vmstat;
    const char *_statFileName;

    // sysfs:
    void update_info(unsigned long long rsum, unsigned long long wsum);
    void getsysfsdiskinfo( void );

    void getdiskinfo( void );
    void getvmdiskinfo( void );
    void updateinfo(unsigned long one, unsigned long two,
      int fudgeFactor);
};


#endif
