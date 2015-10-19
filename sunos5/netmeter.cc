//
//  Copyright (c) 1994, 1995, 2015
//  Rewritten for Solaris by Arno Augustin 1999
//  augustin@informatik.uni-erlangen.de
//
//  This file may be distributed under terms of the GPL
//

#include "netmeter.h"

#include <iostream>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <net/if.h>



NetMeter::NetMeter( kstat_ctl_t *kc )
    : ComNetMeter(),
      _lastBytesIn(0), _lastBytesOut(0),
      _kc(kc),
      _nets(KStatList::getList(_kc, KStatList::NETS)),
      _ignored(false),
      _socket(socket(AF_INET, SOCK_DGRAM, 0)) {

    if ( _socket < 0 )
        logFatal << "Opening socket failed." << std::endl;
}


NetMeter::~NetMeter( void ){
    close(_socket);
}


void NetMeter::checkResources(const ResDB &rdb) {

    ComNetMeter::checkResources(rdb);

    _netIface = rdb.getResource("netIface");
    if (_netIface[0] == '-') {
        _ignored = true;
        _netIface.erase(0, _netIface.find_first_not_of("- "));
    }
}


std::pair<float, float> NetMeter::getRates(void) {
    uint64_t nowBytesIn = 0, nowBytesOut = 0;
    kstat_named_t *k;
    kstat_t *ksp;
    struct lifreq lfr;
    _nets->update(_kc);

    IntervalTimerStop();
    for (unsigned int i = 0; i < _nets->count(); i++) {

        ksp = (*_nets)[i];
        if ( _netIface != "False" &&
          ( (!_ignored && ksp->ks_name != _netIface) ||
            ( _ignored && ksp->ks_name == _netIface) ) )
            continue;
        if ( kstat_read(_kc, ksp, NULL) == -1 )
            continue;

        logDebug << ksp->ks_name << ": \n";

        // try 64-bit byte counter, then 32-bit one, then packet counter
        if ( (k = (kstat_named_t *)kstat_data_lookup(ksp,
              const_cast<char *>("rbytes64"))) == NULL ) {
            if ( (k = (kstat_named_t *)kstat_data_lookup(ksp,
                  const_cast<char *>("rbytes"))) == NULL ) {
                if ( (k = (kstat_named_t *)kstat_data_lookup(ksp,
                      const_cast<char *>("ipackets"))) == NULL )
                    continue;
                // for packet counter, mtu is needed
                std::string(ksp->ks_name).copy(lfr.lifr_name,
                  sizeof(lfr.lifr_name));
                if ( ioctl(_socket, SIOCGLIFMTU, (caddr_t)&lfr) < 0 )
                    continue;
                // not exactly, but must do
                nowBytesIn += kstat_to_ui64(k) * lfr.lifr_mtu;

                logDebug << kstat_to_ui64(k) << " packets received\n";
            }
            else {
                nowBytesIn += kstat_to_ui64(k);

                logDebug << kstat_to_ui64(k) << " bytes received\n";
            }
        }
        else {
            nowBytesIn += kstat_to_ui64(k);

            logDebug << kstat_to_ui64(k) << " bytes received\n";
        }

        if ( (k = (kstat_named_t *)kstat_data_lookup(ksp,
              const_cast<char *>("obytes64"))) == NULL ) {
            if ( (k = (kstat_named_t *)kstat_data_lookup(ksp,
                  const_cast<char *>("obytes"))) == NULL ) {
                if ( (k = (kstat_named_t *)kstat_data_lookup(ksp,
                      const_cast<char *>("opackets"))) == NULL )
                    continue;
                std::string(ksp->ks_name).copy(lfr.lifr_name,
                  sizeof(lfr.lifr_name));
                if ( ioctl(_socket, SIOCGLIFMTU, (caddr_t)&lfr) < 0 )
                    continue;
                nowBytesOut += kstat_to_ui64(k) * lfr.lifr_mtu;

                logDebug << kstat_to_ui64(k) << " packets sent\n";
            }
            else {
                nowBytesOut += kstat_to_ui64(k);

                logDebug << kstat_to_ui64(k) << " bytes sent\n";
            }
        }
        else {
            nowBytesOut += kstat_to_ui64(k);

            logDebug << kstat_to_ui64(k) << " bytes sent\n";
        }
    }

    uint64_t correction = 0x10000000;
    correction *= 0x10;
    /*  Deal with 32-bit wrap by making last value 2^32 less.  Yes,
     *  this is a better idea than adding to nowBytesIn -- the
     *  latter would only work for the first wrap (1+2^32 vs. 1)
     *  but not for the second (1+2*2^32 vs. 1) -- 1+2^32 -
     *  (1+2^32) is still too big.  */
    if (nowBytesIn < _lastBytesIn)
        _lastBytesIn -= correction;
    if (nowBytesOut < _lastBytesOut)
        _lastBytesOut -= correction;
    if(_lastBytesIn == 0)
        _lastBytesIn = nowBytesIn;
    if(_lastBytesOut == 0)
        _lastBytesOut = nowBytesOut;

    double t = IntervalTimeInSecs();
    std::pair<float, float> rval((nowBytesIn - _lastBytesIn) / t,
      (nowBytesOut - _lastBytesOut) / t);

    IntervalTimerStart();
    _lastBytesIn = nowBytesIn;
    _lastBytesOut = nowBytesOut;

    return rval;
}
