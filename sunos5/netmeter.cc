//
//  Copyright (c) 1994, 1995, 2015
//  by Mike Romberg (mike-romberg@comcast.net)
//
//  Rewritten for Solaris by Arno Augustin 1999
//  augustin@informatik.uni-erlangen.de
//
//  This file may be distributed under terms of the GPL
//
#include "netmeter.h"

#include <kstat.h>

static const size_t NNETS = 100;
static const size_t GUESS_MTU = 1500;

NetMeter::NetMeter( XOSView *parent, kstat_ctl_t *_kc, float max )
    : FieldMeterGraph( parent, 3, "NET", "IN/OUT/IDLE" ),
      maxpackets_(max),
      _lastBytesIn(0), _lastBytesOut(0), nnet(0), kc(_kc),
      nnets(NNETS, 0), packetsize(NNETS, 0) {

    kstat_named_t     *k;
    for (kstat_t *ksp = kc->kc_chain; ksp != NULL && nnet <NNETS;
         ksp = ksp->ks_next) {
        if (ksp->ks_type == KSTAT_TYPE_NAMED ){
            if(kstat_read(kc, ksp, NULL) != -1
              && std::string(ksp->ks_name).substr(0, 3) != "lo0") {

                k = (kstat_named_t *)(ksp->ks_data);
                if (std::string(k->name).substr(0, 8) == "ipackets") {
                    nnets[nnet] = ksp;
                    packetsize[nnet] = GUESS_MTU;
                    // check if bytes r/w also available
                    for(unsigned int j=0; j< ksp->ks_ndata; j++, k++) {
                        // search for byte fields
                        if (std::string(k->name).substr(0, 6) == "obytes") {
                            packetsize[nnet] = 1;
                        }
                    }
                    logDebug << "Found NET: " << ksp->ks_name
                             << (packetsize[nnet] == 1
                               ? "(Bytes)" : "(Packets Only)") << std::endl;
                    nnet++;
                }
            }
        }

    }
    IntervalTimerStart();
    total_ = max;
    _lastBytesIn = _lastBytesOut = 0;
}

NetMeter::~NetMeter( void ){
}

void NetMeter::checkResources( void ){

    FieldMeterGraph::checkResources();

    setfieldcolor( 0, parent_->getResource("netInColor") );
    setfieldcolor( 1, parent_->getResource("netOutColor") );
    setfieldcolor( 2, parent_->getResource("netBackground") );
    priority_ = util::stoi (parent_->getResource("netPriority") );
    dodecay_ = parent_->isResourceTrue("netDecay");
    useGraph_ = parent_->isResourceTrue("netGraph");
    setUsedFormat (parent_->getResource("netUsedFormat"));
}

void NetMeter::checkevent( void ){

    //  Reset total_ to expected maximum.  If it is too low, it
    //  will be adjusted in adjust().  bgrayson
    total_ = maxpackets_;

    fields_[0] = fields_[1] = 0;
    long long nowBytesIn=0, nowBytesOut=0, mtu;

    IntervalTimerStop();
    kstat_named_t     *k;
    for (size_t i = 0; i < nnet; i++) { // see man kstat......
        kstat_t *ksp=nnets[i];
        k = (kstat_named_t *)(ksp->ks_data);
        if (kstat_read(kc, ksp, 0) == -1)
            continue;

        mtu = packetsize[i];
        for(unsigned int j=0, found=0; j< ksp->ks_ndata && found<2; j++,k++){
            std::string k_name(k->name);
            if (k_name.substr(0, 5) == (mtu == 1 ? "rbyte" : "ipack")) {
                nowBytesIn += k->value.ul*mtu;
                found++;
            }
            else if (k_name.substr(0, 5) == (mtu == 1 ? "obyte" : "opack")) {
                nowBytesOut += k->value.ul*mtu;
                found++;
            }
        }
    }

    long long correction = 0x10000000;
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
    float t = (1.0) / IntervalTimeInSecs();
    fields_[0] = (float)(nowBytesIn - _lastBytesIn) * t;
    _lastBytesIn = nowBytesIn;
    fields_[1] = (float)(nowBytesOut - _lastBytesOut) * t;
    _lastBytesOut = nowBytesOut;
    //  End BSD-specific code.  BCG

    adjust();
    fields_[2] = total_ - fields_[0] - fields_[1];
    /*  The fields_ values have already been scaled into bytes/sec by
     *  the manipulations (* t) above.  */
    setUsed (fields_[0]+fields_[1], total_);
    IntervalTimerStart();
    drawfields(parent_->g());
}

void NetMeter::adjust(void){
    if (total_ < (fields_[0] + fields_[1]))
        total_ = fields_[0] + fields_[1];
}
