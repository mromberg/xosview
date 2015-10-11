//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef SERIALMETER_H
#define SERIALMETER_H

#include "bitmeter.h"



class SerialMeter : public BitMeter {
public:
    enum Device { S0, S1, S2, S3, S4, S5, S6, S7, S8, S9 };
    static int numDevices(void) { return 10; }

    SerialMeter( XOSView *parent, Device device);
    ~SerialMeter( void );

    static std::string getResourceName(Device dev);

    void checkevent( void );

    virtual std::string resName(void) const { return "serial"; }
    void checkResources(const ResDB &rdb);

private:
    unsigned short int _port;
    Device _device;

    void getserial( void );
    bool getport(unsigned short int port);
    std::string getTitle(Device dev) const;
    unsigned short int getPortBase(const ResDB &rdb, Device dev) const;
};


#endif
