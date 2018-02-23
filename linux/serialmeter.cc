//
//  Copyright (c) 1994, 1995, 2006, 2015, 2016, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
//-----------------------------------------------------------------------
//  In order to use this new serial meter, xosview needs to be suid root.
//-----------------------------------------------------------------------
//
#include "serialmeter.h"
#include "strutil.h"

#include <iomanip>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
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


// hack for not having linux/serial_reg.h, (Debian bug #427599)
#ifndef UART_LSR
static const size_t UART_LSR = 5;
#endif
#ifndef UART_MSR
static const size_t UART_MSR = 6;
#endif



SerialMeter::SerialMeter(Device device)
    : BitMeter(getTitle(device), "LSR bits(0-7), MSR bits(0-7)", 16),
      _port(0), _device(device) {
}


void SerialMeter::checkevent(void) {
    getserial();
}


void SerialMeter::checkResources(const ResDB &rdb) {
    BitMeter::checkResources(rdb);
    _dbits.color(0, rdb.getColor("serialOffColor"));
    _dbits.color(1, rdb.getColor("serialOnColor"));

    _port = getPortBase(rdb, _device);
    if (!getport(_port + UART_LSR) || !getport(_port + UART_MSR)) {
        logFatal << "SerialMeter::SerialMeter() : "
                 << "xosview must be suid root to use the serial meter."
                 << std::endl;
    }
}


bool SerialMeter::getport(unsigned short int port) {
#ifdef HAVE_IOPERM
    return ioperm(port, 1, 1) != -1;
#else
    return false;
#endif
}


void SerialMeter::getserial(void) {
#ifdef HAVE_IOPERM
    // get the LSR and MSR
    unsigned char lsr = inb(_port + UART_LSR);
    unsigned char msr = inb(_port + UART_MSR);

    setBits(0, lsr);
    setBits(8, msr);
#endif
}


std::string SerialMeter::getTitle(Device dev) const {
    return "ttyS" + std::to_string(static_cast<unsigned>(dev));
}


std::string SerialMeter::getResourceName(Device dev) {
    return "serial" + std::to_string(static_cast<unsigned>(dev));
}


unsigned short int SerialMeter::getPortBase(const ResDB &rdb,
  Device dev) const {

    const std::string deviceFile = "/dev/ttyS"
        + std::to_string(static_cast<unsigned>(dev));
    const std::string res = rdb.getResource(getResourceName(dev));

    if (util::tolower(res) == "true") { // Autodetect portbase.
        // get the real serial port (code stolen from setserial 2.11)
        int fd;
        if ((fd = open(deviceFile.c_str(), O_RDONLY | O_NONBLOCK)) < 0) {
            logFatal << "SerialMeter::SerialMeter() : "
                     << "failed to open " << deviceFile[dev] <<"."
                     << std::endl;
        }
        struct serial_struct serinfo;
        if (ioctl(fd, TIOCGSERIAL, &serinfo) < 0) {
            close(fd);
            logFatal << "Failed to detect port base for " << deviceFile[dev]
                     << std::endl;
        }

        close(fd);
        return serinfo.port;
    }
    else { // Use user specified port base.
        std::istringstream istrm(res);
        unsigned short int tmp = 0;
        istrm >> std::hex >> tmp;
        return tmp;
    }

    return 0;
}
