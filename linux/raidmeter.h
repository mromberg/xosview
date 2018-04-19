//
//  Copyright (c) 1999, 2006, 2015, 2016, 2018
//  by Thomas Waldmann ( ThomasWaldmann@gmx.de )
//  based on work of Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef raidmeter_h
#define raidmeter_h

#include "bitfieldmeter.h"


class RAIDMeter : public BitFieldMeter {
public:
    RAIDMeter(const std::string &device);

    virtual void checkevent(void) override;
    virtual std::string resName(void) const override { return "RAID"; }
    virtual void checkResources(const ResDB &rdb) override;

    static std::vector<std::string> devices(const ResDB &rdb);

private:
    std::string _device;
    std::string _dir;
    std::string _level;
    size_t _ffsize;
    unsigned long _ffColor;
    unsigned long _degradedColor;
    std::map<std::string, unsigned long> _actionColors;
    std::map<std::string, unsigned long> _driveColors;

    std::vector<std::string> scanDevs(void) const; // list devices in _dir.
    size_t setDevBits(void);         // returns number of active devices.
    std::string setSyncAction(void); // returns current sync_action.
    std::string filterState(const std::string &state) const;

    static std::vector<std::string> scanMDDevs(void);
    static const std::map<std::string, unsigned char> &devState(void);
};


#endif
