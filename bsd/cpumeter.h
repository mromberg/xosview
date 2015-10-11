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

#ifndef CPUMETER_H
#define CPUMETER_H

#include "fieldmetergraph.h"



class CPUMeter : public FieldMeterGraph {
public:
    CPUMeter( XOSView *parent, unsigned int nbr );
    ~CPUMeter( void );

    virtual std::string resName( void ) const { return "cpu"; }
    void checkevent( void );
    void checkResources(const ResDB &rdb);

protected:
    void getcputime( void );

private:
    std::vector<std::vector<uint64_t> > cputime_;
    unsigned int cpuindex_, nbr_;
};


#endif
