//
//  Copyright (c) 1994, 1995, 2006, 2015, 2016
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
//-----------------------------------------------------------------------
//  In order to use this new serial meter, xosview needs to be suid root.
//-----------------------------------------------------------------------
//
#include "serialmeter.h"

#include <sstream>
#include <iomanip>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>


// hack for not having linux/serial_reg.h, (Debian bug #427599)
#ifndef UART_LSR
#define UART_LSR        5
#endif
#ifndef UART_MSR
#define UART_MSR        6
#endif


#ifdef HAVE_SYS_IO_H
#include <sys/io.h>
#endif

#ifdef HAVE_SYS_PERM_H
#include <sys/perm.h>
#endif

#ifdef HAVE_ASM_IO_H
#include <asm/io.h>
#endif

#include <linux/serial.h>



SerialMeter::SerialMeter( Device device )
    : BitMeter( getTitle(device), "LSR bits(0-7), MSR bits(0-7)", 16){
    _device = device;
    _port = 0;
}


SerialMeter::~SerialMeter( void ){
}


void SerialMeter::checkevent( void ){
    getserial();
}


void SerialMeter::checkResources(const ResDB &rdb){
    BitMeter::checkResources(rdb);
    _dbits.color(0, rdb.getColor("serialOffColor"));
    _dbits.color(1, rdb.getColor("serialOnColor"));

    _port = getPortBase(rdb, _device);
    if (!getport(_port + UART_LSR) || !getport(_port + UART_MSR)){
        logFatal << "SerialMeter::SerialMeter() : "
                 << "xosview must be suid root to use the serial meter."
                 << std::endl;
    }
}


bool SerialMeter::getport(unsigned short int port){
#ifdef HAVE_IOPERM
    return ioperm(port, 1, 1) != -1;
#else
    return -1 != -1;
#endif
}


void SerialMeter::getserial( void ){
#ifdef HAVE_IOPERM
    // get the LSR and MSR
    unsigned char lsr = inb(_port + UART_LSR);
    unsigned char msr = inb(_port + UART_MSR);

    setBits(0, lsr);
    setBits(8, msr);
#endif
}


std::string SerialMeter::getTitle(Device dev) const {
    static const char *names[] = { "ttyS0", "ttyS1", "ttyS2", "ttyS3",
                                   "ttyS4", "ttyS5", "ttyS6", "ttyS7",
                                   "ttyS8", "ttyS9" };
    return names[dev];
}


std::string SerialMeter::getResourceName(Device dev){
    static const char *names[] = { "serial0", "serial1",
                                   "serial2", "serial3",
                                   "serial4", "serial5",
                                   "serial6", "serial7",
                                   "serial8", "serial9" };

    return names[dev];
}


unsigned short int SerialMeter::getPortBase(const ResDB &rdb,
  Device dev) const {

    static const char *deviceFile[] = { "/dev/ttyS0",
                                        "/dev/ttyS1",
                                        "/dev/ttyS2",
                                        "/dev/ttyS3",
                                        "/dev/ttyS4",
                                        "/dev/ttyS5",
                                        "/dev/ttyS6",
                                        "/dev/ttyS7",
                                        "/dev/ttyS8",
                                        "/dev/ttyS9"};

    std::string res = rdb.getResource(getResourceName(dev));

    if (util::tolower(res) == "true"){ // Autodetect portbase.
        int fd;
        struct serial_struct serinfo;

        // get the real serial port (code stolen from setserial 2.11)
        if ((fd = open(deviceFile[dev], O_RDONLY|O_NONBLOCK)) < 0) {
            logFatal << "SerialMeter::SerialMeter() : "
                     << "failed to open " << deviceFile[dev] <<"."
                     << std::endl;
        }
        if (ioctl(fd, TIOCGSERIAL, &serinfo) < 0) {
            close(fd);
            logFatal << "Failed to detect port base for " << deviceFile[dev]
                     << std::endl;
        }

        close(fd);
        return serinfo.port;
    }
    else { // Use user specified port base.
        std::string s(res);
        std::istringstream istrm(s);
        unsigned short int tmp = 0;
        istrm >> std::hex >> tmp;
        return tmp;
    }

    return 0;
}
