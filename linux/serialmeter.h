//
//  Copyright (c) 1994, 1995, 2006, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef serialmeter_h
#define serialmeter_h

#include "bitmeter.h"



class SerialMeter : public BitMeter {
public:
    enum Device { S0, S1, S2, S3, S4, S5, S6, S7, S8, S9 };

    SerialMeter(Device device);
    virtual ~SerialMeter(void);

    static size_t numDevices(void) { return 10; }
    static std::string getResourceName(Device dev);

    virtual void checkevent(void) override;

    virtual std::string resName(void) const override { return "serial"; }
    virtual void checkResources(const ResDB &rdb) override;

private:
    unsigned short int _port;
    Device _device;

    void getserial(void);
    bool getport(unsigned short int port);
    std::string getTitle(Device dev) const;
    unsigned short int getPortBase(const ResDB &rdb, Device dev) const;
};


#endif
