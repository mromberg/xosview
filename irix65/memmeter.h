//
//  Copyright (c) 1994, 1995, 2004, 2006, 2015
//  Initial port performed by Stefan Eilemann (eilemann@gmail.com)
//
//  This file may be distributed under terms of the GPL
//
#ifndef MEMMETER_H
#define MEMMETER_H

#include "fieldmetergraph.h"

#include <sys/types.h>
#include <sys/sysmp.h>
#include <sys/sysinfo.h> /* for SAGET and MINFO structures */

class MemMeter : public FieldMeterGraph {
public:
    MemMeter(XOSView *parent);
    ~MemMeter(void);

    virtual std::string name(void) const { return "MemMeter"; }
    void checkevent( void );

    void checkResources(void);

protected:
    //  struct pst_status *stats_;
    int _pageSize;

    void getmeminfo( void );

private:
    struct rminfo  mp;
    int            minfosz;
};

#endif
