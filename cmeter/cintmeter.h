//
//  Copyright (c) 2017, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef cintmeter_h
#define cintmeter_h

#include "bitmeter.h"



class ComIntMeter : public BitMeter {
public:
    virtual void checkevent(void) override;
    virtual std::string resName(void) const override { return "int"; }
    virtual void checkResources(const ResDB &rdb) override;

protected:
    ComIntMeter(const std::string &title);

    virtual const std::map<size_t, uint64_t> &getStats(void) = 0;

private:
    std::map<size_t, uint64_t> _last; // counts from last event.
    std::map<size_t, size_t> _imap;   // irq -> bit index.

    void initUI(void);
    void initIMap(void);
    std::string makeLegend(void) const;
};

#endif
