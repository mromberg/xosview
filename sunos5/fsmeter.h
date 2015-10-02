//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#ifndef FSMETER_H
#define FSMETER_H

#include "cfsmeter.h"



class FSMeter : public ComFSMeter {
public:
    FSMeter(XOSView *parent, const std::string &path)
        : ComFSMeter(parent, path) {}

protected:
    virtual bool isMount(const std::string &path);
};


class FSMeterFactory : public FSMFactory<FSMeter> {
public:
    virtual std::vector<std::string> getAuto(void);
};

#undef MOUNT_FNAME

#endif // end FSMETER_H
