//
//  Copyright (c) 2017
//  by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  This file may be distributed under terms of the GPL
//

#ifndef CINTMETER_H
#define CINTMETER_H

#include "bitmeter.h"

#include <stdint.h>



class ComIntMeter : public BitMeter {
public:

    virtual void checkevent(void);
    virtual std::string resName(void) const { return "int"; }
    virtual void checkResources(const ResDB &rdb);

protected:
    ComIntMeter(const std::string &title);

    virtual std::map<size_t, uint64_t> getStats(void) const = 0;

private:
    std::map<size_t, uint64_t> _last; // counts from last event.
    std::map<size_t, size_t> _imap;   // irq -> bit index.

    void initUI(void);
    void initIMap(void);
    std::string makeLegend(void) const;
};

#endif
