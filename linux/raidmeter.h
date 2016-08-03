//
//  Copyright (c) 1999, 2006, 2015, 2016
//  by Thomas Waldmann ( ThomasWaldmann@gmx.de )
//  based on work of Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef RAIDMETER_H
#define RAIDMETER_H

#include "bitfieldmeter.h"


class RAIDMeter : public BitFieldMeter {
public:
    RAIDMeter(const std::string &device);
    ~RAIDMeter(void);

    void checkevent(void);

    virtual std::string resName(void) const { return "RAID"; }

    void checkResources(const ResDB &rdb);

    static std::vector<std::string> devices(const ResDB &rdb);

private:
    std::string _device;
    std::string _dir;
    std::string _level;
    size_t _ffsize;
    std::map<std::string, unsigned long> _actionColors;
    std::map<std::string, unsigned long> _driveColors;

    std::vector<std::string> scanDevs(void); // list of devices found in _dir.
    size_t setDevBits(void);         // returns number of active devices.
    std::string setSyncAction(void); // returns current sync_action.
    std::string filterState(const std::string &state) const;
    static std::vector<std::string> scanMDDevs(void);
    static const std::map<std::string, unsigned char> &devState(void);
};


#endif
