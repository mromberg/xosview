//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#ifndef fsmeter_h
#define fsmeter_h

#include "cfsmeter.h"



class FSMeter : public ComFSMeter {
public:
    FSMeter(const std::string &path) : ComFSMeter(path) {}

protected:
    virtual bool isMount(const std::string &path) override;
};


class FSMeterFactory : public FSMFactory<FSMeter> {
public:
    virtual std::vector<std::string> getAuto(void);
};


#endif // end FSMETER_H
